/*
 *  LLVM FunctionWrapper Pass
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
#include "FunctionWrapper.h"

static cl::opt<int> ProbRate("fw_prob",cl::desc("Choose the probability [%] For Each CallSite To Be Obfuscated By FunctionWrapper"),cl::value_desc("Probability Rate"), cl::init(70), cl::Optional);

static cl::opt<int> ObfTimes("fw_times",cl::desc("Choose how many time the FunctionWrapper pass loop on a CallSite"),cl::value_desc("Number of Times"), cl::init(3), cl::Optional);

PreservedAnalyses FunctionWrapperPass::run(Module &M, ModuleAnalysisManager& AM) {
    vector<CallSite *> callsites;
    for (Module::iterator iter = M.begin(); iter != M.end(); iter++) {
      Function &F = *iter; // 迭代每个函数
      if (toObfuscate(flag, &F, "funwra")) {
        outs() << "\033[1;32m[FunctionWrapper] Function : " << F.getName() << "\033[0m\n"; // 打印一下被混淆函数的symbol
        for (inst_iterator fi = inst_begin(&F); fi != inst_end(&F); fi++) {
          Instruction *Inst = &*fi;
          if (isa<CallInst>(Inst) || isa<InvokeInst>(Inst)) {
            if ((int)llvm::cryptoutils->get_range(100) <= ProbRate) {
                callsites.push_back(new CallSite(Inst));
            }
          }
        }
      }
    }
    for (CallSite *CS : callsites){ // 嵌套混淆发生在全局 而不是针对某个函数
      for (int i = 0; i < ObfTimes && CS != nullptr; i++){
        CS = HandleCallSite(CS);
      }
    }
    return PreservedAnalyses::all();
}

CallSite* FunctionWrapperPass::HandleCallSite(CallSite *CS) {
    Value *calledFunction = CS->getCalledFunction();
    if (calledFunction == nullptr) {
      calledFunction = CS->getCalledValue()->stripPointerCasts();
    }
    // Filter out IndirectCalls that depends on the context
    // Otherwise It'll be blantantly troublesome since you can't reference an
    // Instruction outside its BB  Too much trouble for a hobby project
    // To be precise, we only keep CS that refers to a non-intrinsic function
    // either directly or through casting
    if (calledFunction == nullptr || (!isa<ConstantExpr>(calledFunction) && !isa<Function>(calledFunction)) || CS->getIntrinsicID() != Intrinsic::not_intrinsic) {
        return nullptr;
    }
    if (Function *tmp = dyn_cast<Function>(calledFunction)) {
        if (tmp->getName().startswith("clang.")) {
            // Clang Intrinsic
            return nullptr;
        }
    }
    // Create a new function which in turn calls the actual function
    // 创建一个新的函数 return的时候返回真实函数 此所谓函数嵌套
    vector<Type *> types;
    for (unsigned i = 0; i < CS->getNumArgOperands(); i++) {
      types.push_back(CS->getArgOperand(i)->getType());
    }
    FunctionType *ft = FunctionType::get(CS->getType(), ArrayRef<Type *>(types), false);
    // Function *func = Function::Create(ft, GlobalValue::LinkageTypes::InternalLinkage, "O0ooOO0o0OO0oO", CS->getParent()->getModule()); // 移除Hikari特征
    // std::string randstring = rand_str((rand() % (20 - 5 + 1)) + 5); // 生成 长度随机 的ooOO00
    std::string randstring = rand_str(20); // 生成 长度固定 的ooOO00
    Function *func = Function::Create(ft, GlobalValue::LinkageTypes::InternalLinkage, Twine("Hack" + randstring + "END"), CS->getParent()->getModule()); // 优化为随机字符串 // FIXBUGS
    appendToCompilerUsed(*func->getParent(), {func});
    // FIXME: Correctly Steal Function Attributes
    // func->addFnAttr(Attribute::AttrKind::OptimizeNone);
    // func->addFnAttr(Attribute::AttrKind::NoInline);
    func->copyAttributesFrom(cast<Function>(calledFunction));
    BasicBlock *BB = BasicBlock::Create(func->getContext(), "", func);
    IRBuilder<> IRB(BB);
    vector<Value *> params;
    for (auto arg = func->arg_begin(); arg != func->arg_end(); arg++) {
      params.push_back(arg);
    }
    Value *retval = IRB.CreateCall(cast<FunctionType>(ConstantExpr::getBitCast(cast<Function>(calledFunction), CS->getCalledValue()->getType())->getType()->getPointerElementType()), ConstantExpr::getBitCast(cast<Function>(calledFunction), CS->getCalledValue()->getType()), ArrayRef<Value *>(params));
    if (ft->getReturnType()->isVoidTy()) {
      IRB.CreateRetVoid();
    } else {
      IRB.CreateRet(retval);
    }
    CS->setCalledFunction(func);
    CS->mutateFunctionType(ft);
    Instruction *Inst = CS->getInstruction();
    delete CS;
    return new CallSite(Inst);
}

/**
 * @brief 便于调用函数包装器
 *
 * @param flag
 * @return FunctionPass*
 */
FunctionWrapperPass *llvm::createFunctionWrapper(bool flag){
    return new FunctionWrapperPass(flag);
}