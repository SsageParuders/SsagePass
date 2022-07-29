/*
    LLVM Indirect Branching Pass
    Copyright (C) 2017 Zhang(https://github.com/Naville/)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "IndirectBranch.h"

PreservedAnalyses IndirectBranchPass::run(Module &M, ModuleAnalysisManager& AM) {
    vector<Function *> funcs;
    for (Module::iterator iter = M.begin(); iter != M.end(); iter++) {
        funcs.push_back(&*iter);
    }
    for (Function *F : funcs) {
        if (toObfuscate(flag, F, "indibr")) {
            outs() << "\033[44;37m============IndirectBranch Start============\033[0m\n";
            outs() << "\033[42;35mFunction : " << F->getName() << "\033[0m\n"; // 打印一下被混淆函数的symbol
            HandleFunction(*F);
            outs() << "\033[44;37m============IndirectBranch Finish============\033[0m\n";
        }
    }
    return PreservedAnalyses::all();
}

bool IndirectBranchPass::HandleFunction(Function &Func){ 
    if (this->initialized == false) {
      initialize(*Func.getParent());
      this->initialized = true;
    }
    vector<BranchInst *> BIs;
    for (inst_iterator I = inst_begin(Func); I != inst_end(Func); I++) {
      Instruction *Inst = &(*I);
      if (BranchInst *BI = dyn_cast<BranchInst>(Inst)) {
        BIs.push_back(BI);
      }
    } // Finish collecting branching conditions
    Value *zero = ConstantInt::get(Type::getInt32Ty(Func.getParent()->getContext()), 0);
    for (BranchInst *BI : BIs) {
        IRBuilder<> IRB(BI);
        vector<BasicBlock *> BBs;
        // We use the condition's evaluation result to generate the GEP
        // instruction  False evaluates to 0 while true evaluates to 1.  So here
        // we insert the false block first
        if (BI->isConditional()) {
            BBs.push_back(BI->getSuccessor(1));
        }
        BBs.push_back(BI->getSuccessor(0));
        ArrayType *AT = ArrayType::get(Type::getInt8PtrTy(Func.getParent()->getContext()), BBs.size());
        vector<Constant *> BlockAddresses;
        for (unsigned i = 0; i < BBs.size(); i++) {
            BlockAddresses.push_back(BlockAddress::get(BBs[i]));
        }
        GlobalVariable *LoadFrom = NULL;

        if (BI->isConditional() || indexmap.find(BI->getSuccessor(0)) == indexmap.end()) {
            // Create a new GV
            Constant *BlockAddressArray = ConstantArray::get(AT, ArrayRef<Constant *>(BlockAddresses));
            LoadFrom = new GlobalVariable(*Func.getParent(), AT, false, GlobalValue::LinkageTypes::PrivateLinkage, BlockAddressArray, "HikariConditionalLocalIndirectBranchingTable");
            appendToCompilerUsed(*Func.getParent(), {LoadFrom});
        } else {
            LoadFrom = Func.getParent()->getGlobalVariable("IndirectBranchingGlobalTable", true);
        }
        Value *index = NULL;
        if (BI->isConditional()) {
            Value *condition = BI->getCondition();
            index = IRB.CreateZExt(condition, Type::getInt32Ty(Func.getParent()->getContext()));
        } else {
            index = ConstantInt::get(Type::getInt32Ty(Func.getParent()->getContext()), indexmap[BI->getSuccessor(0)]);
        }
        Value *GEP = IRB.CreateGEP(LoadFrom->getType()->getPointerElementType(), LoadFrom, {zero, index});
        LoadInst *LI = IRB.CreateLoad(GEP->getType()->getPointerElementType(), GEP, "IndirectBranchingTargetAddress");
        IndirectBrInst *indirBr = IndirectBrInst::Create(LI, BBs.size());
        for (BasicBlock *BB : BBs){
            indirBr->addDestination(BB);
        }
        ReplaceInstWithInst(BI, indirBr);
    }
    return true;
}

bool IndirectBranchPass::initialize(Module & M){
    vector<Constant *> BBs;
    unsigned long long i = 0;
    for (auto F = M.begin(); F != M.end(); F++){
        for (auto BB = F->begin(); BB != F->end(); BB++){
            BasicBlock *BBPtr = &*BB;
            if (BBPtr != &(BBPtr->getParent()->getEntryBlock())){
                indexmap[BBPtr] = i++;
                BBs.push_back(BlockAddress::get(BBPtr));
            }
        }
    }
    ArrayType *AT = ArrayType::get(Type::getInt8PtrTy(M.getContext()), BBs.size());
    Constant *BlockAddressArray = ConstantArray::get(AT, ArrayRef<Constant *>(BBs));
    GlobalVariable *Table = new GlobalVariable(M, AT, false, GlobalValue::LinkageTypes::InternalLinkage, BlockAddressArray, "IndirectBranchingGlobalTable");
    appendToCompilerUsed(M, {Table});
    return true;
}

bool IndirectBranchPass::doFinalization(Module & M){
    indexmap.clear();
    initialized = false;
    return false;
}

/**
 * @brief 便于调用间接指令
 *
 * @param flag
 * @return FunctionPass*
 */
IndirectBranchPass *llvm::createIndirectBranch(bool flag){
    return new IndirectBranchPass(flag);
}