#include "StringEncryption.h"

#define DEBUG_TYPE "strenc"

using namespace llvm;

PreservedAnalyses StringEncryptionPass::run(Module &M, ModuleAnalysisManager& AM) {
    // in runOnModule. We simple iterate function list and dispatch functions to handlers
    // 在runOnModule 我们简单地迭代函数列表并将函数分派给处理程序
    for (Module::iterator iter = M.begin(); iter != M.end(); iter++){
        Function *F = &(*iter);
        if (toObfuscate(flag, F, "strenc")){
            outs() << "\033[1;34m============StringEncryption Start============\033[0m\n";
            outs() << "\033[1;32mFunction : " << F->getName() << "\033[0m\n"; // 打印一下被混淆函数的symbol
            Constant *S = ConstantInt::get(Type::getInt32Ty(M.getContext()), 0);
            GlobalVariable *GV = new GlobalVariable(M, S->getType(), false, GlobalValue::LinkageTypes::PrivateLinkage, S, "");
            encstatus[F] = GV;
            HandleFunction(F);
            outs() << "\033[1;34m============StringEncryption Finish============\033[0m\n";
        }
    }
    return PreservedAnalyses::all();
}

void StringEncryptionPass::HandleFunction(Function *Func) {
    /*
        - Split Original EntryPoint BB into A and C.
        - Create new BB as Decryption BB between A and C. Adjust the terminators
        into: A (Alloca a new array containing all)
                |
                B(If not decrypted)
                |
                C
    */
    FixFunctionConstantExpr(Func);
    BasicBlock *A = &(Func->getEntryBlock());
    BasicBlock *C = A->splitBasicBlock(A->getFirstNonPHIOrDbgOrLifetime());
    C->setName("PrecedingBlock");
    BasicBlock *B = BasicBlock::Create(Func->getContext(), "StringDecryptionBB", Func, C);
    // Change A's terminator to jump to B
    // We'll add new terminator to jump C later
    BranchInst *newBr = BranchInst::Create(B);
    ReplaceInstWithInst(A->getTerminator(), newBr);
    set<GlobalVariable *> Globals;
    set<User *> Users;
    for (BasicBlock &BB : *Func) {
      for (Instruction &I : BB) {
        for (Value *Op : I.operands()) {
          if (GlobalVariable *G = dyn_cast<GlobalVariable>(Op->stripPointerCasts())) {
            if(User* U=dyn_cast<User>(Op)){
              Users.insert(U);
            }
            Users.insert(&I);
            Globals.insert(G);
          }
        }
      }
    }
    IRBuilder<> IRB(A->getFirstNonPHIOrDbgOrLifetime());
    set<GlobalVariable *> rawStrings;
    set<GlobalVariable *> objCStrings;
    map<GlobalVariable *, Constant *> GV2Keys;
    map<GlobalVariable * /*old*/, GlobalVariable * /*new*/> old2new;
    for (GlobalVariable *GV : Globals) {
        if (GV->hasInitializer() && GV->getSection() != StringRef("llvm.metadata") && GV->getSection().find(StringRef("__objc")) == string::npos && GV->getName().find("OBJC") == string::npos) {
            if (GV->getInitializer()->getType() == StructType::getTypeByName(Func->getParent()->getContext(), "struct.__NSConstantString_tag")) {
                objCStrings.insert(GV);
                rawStrings.insert(cast<GlobalVariable>(cast<ConstantStruct>(GV->getInitializer())->getOperand(2)->stripPointerCasts()));
            } else if (isa<ConstantDataSequential>(GV->getInitializer())) {
                rawStrings.insert(GV);
            } else if(isa<ConstantArray>(GV->getInitializer())){
                ConstantArray* CA=cast<ConstantArray>(GV->getInitializer());
                for(unsigned i=0;i<CA->getNumOperands();i++){
                    Value* op=CA->getOperand(i)->stripPointerCasts();
                    if(GlobalVariable* GV=dyn_cast<GlobalVariable>(op)){
                    Globals.insert(GV);
                    }
                }
            }
        }
    }
    for (GlobalVariable *GV : rawStrings){
        if (GV->getInitializer()->isZeroValue() || GV->getInitializer()->isNullValue()){
            continue;
        }
        ConstantDataSequential *CDS = cast<ConstantDataSequential>(GV->getInitializer());
        Type *memberType = CDS->getElementType();
        // Ignore non-IntegerType
        if (!isa<IntegerType>(memberType)){
            continue;
        }
        IntegerType *intType = cast<IntegerType>(memberType);
        Constant *KeyConst = NULL;
        Constant *EncryptedConst = NULL;
        if (intType == Type::getInt8Ty(GV->getParent()->getContext())){
            vector<uint8_t> keys;
            vector<uint8_t> encry;
            for (unsigned i = 0; i < CDS->getNumElements(); i++) {
                uint8_t K = cryptoutils->get_uint8_t();
                uint64_t V = CDS->getElementAsInteger(i);
                keys.push_back(K);
                encry.push_back(K ^ V);
            }
            KeyConst = ConstantDataArray::get(GV->getParent()->getContext(), ArrayRef<uint8_t>(keys));
            EncryptedConst = ConstantDataArray::get(GV->getParent()->getContext(), ArrayRef<uint8_t>(encry));
        } else if (intType == Type::getInt16Ty(GV->getParent()->getContext())){
            vector<uint16_t> keys;
            vector<uint16_t> encry;
            for (unsigned i = 0; i < CDS->getNumElements(); i++){
                uint16_t K = cryptoutils->get_uint16_t();
                uint64_t V = CDS->getElementAsInteger(i);
                keys.push_back(K);
                encry.push_back(K ^ V);
            }
            KeyConst = ConstantDataArray::get(GV->getParent()->getContext(), ArrayRef<uint16_t>(keys));
            EncryptedConst = ConstantDataArray::get(GV->getParent()->getContext(), ArrayRef<uint16_t>(encry));
        } else if (intType == Type::getInt32Ty(GV->getParent()->getContext())) {
            vector<uint32_t> keys;
            vector<uint32_t> encry;
            for (unsigned i = 0; i < CDS->getNumElements(); i++) {
                uint32_t K = cryptoutils->get_uint32_t();
                uint64_t V = CDS->getElementAsInteger(i);
                keys.push_back(K);
                encry.push_back(K ^ V);
            }
            KeyConst = ConstantDataArray::get(GV->getParent()->getContext(), ArrayRef<uint32_t>(keys));
            EncryptedConst = ConstantDataArray::get(GV->getParent()->getContext(), ArrayRef<uint32_t>(encry));
        } else if (intType == Type::getInt64Ty(GV->getParent()->getContext())) {
            vector<uint64_t> keys;
            vector<uint64_t> encry;
            for (unsigned i = 0; i < CDS->getNumElements(); i++) {
                uint64_t K = cryptoutils->get_uint64_t();
                uint64_t V = CDS->getElementAsInteger(i);
                keys.push_back(K);
                encry.push_back(K ^ V);
            }
            KeyConst = ConstantDataArray::get(GV->getParent()->getContext(), ArrayRef<uint64_t>(keys));
            EncryptedConst = ConstantDataArray::get(GV->getParent()->getContext(), ArrayRef<uint64_t>(encry));
        } else {
            errs() << "Unsupported CDS Type\n";
            abort();
        }
        // Prepare new rawGV
        GlobalVariable *EncryptedRawGV = new GlobalVariable(*(GV->getParent()), EncryptedConst->getType(), false, GV->getLinkage(), EncryptedConst, "EncryptedString", nullptr, GV->getThreadLocalMode(), GV->getType()->getAddressSpace());
        old2new[GV] = EncryptedRawGV;
        GV2Keys[EncryptedRawGV] = KeyConst;
    }
    // Now prepare ObjC new GV
    for (GlobalVariable *GV : objCStrings){
        Value *zero = ConstantInt::get(Type::getInt32Ty(GV->getContext()), 0);
        ConstantStruct *CS = cast<ConstantStruct>(GV->getInitializer());
        vector<Constant *> vals;
        vals.push_back(CS->getOperand(0));
        vals.push_back(CS->getOperand(1));
        GlobalVariable *oldrawString = cast<GlobalVariable>(CS->getOperand(2)->stripPointerCasts());
        if (old2new.find(oldrawString) == old2new.end()){ // Filter out zero initializers
            continue;
        }
        Constant *GEPed = ConstantExpr::getInBoundsGetElementPtr(nullptr, old2new[oldrawString], {zero, zero});
        if (GEPed->getType() == CS->getOperand(2)->getType()){
            vals.push_back(GEPed);
        } else {
            Constant *BitCasted = ConstantExpr::getBitCast(old2new[oldrawString], CS->getOperand(2)->getType());
            vals.push_back(BitCasted);
        }
        vals.push_back(CS->getOperand(3));
        Constant *newCS = ConstantStruct::get(CS->getType(), ArrayRef<Constant *>(vals));
        GlobalVariable *EncryptedOCGV = new GlobalVariable(*(GV->getParent()), newCS->getType(), false, GV->getLinkage(), newCS, "EncryptedObjCString", nullptr, GV->getThreadLocalMode(), GV->getType()->getAddressSpace());
        old2new[GV] = EncryptedOCGV;
    } // End prepare ObjC new GV
    // Replace Uses
    for (User *U : Users) {
      for (map<GlobalVariable *, GlobalVariable *>::iterator iter =
               old2new.begin();
           iter != old2new.end(); ++iter) {
        U->replaceUsesOfWith(iter->first, iter->second);
        iter->first->removeDeadConstantUsers();
      }
    } // End Replace Uses
    // CleanUp Old ObjC GVs
    for (GlobalVariable *GV : objCStrings) {
      if (GV->getNumUses() == 0) {
        GV->dropAllReferences();
        old2new.erase(GV);
        GV->eraseFromParent();
      }
    }
    // CleanUp Old Raw GVs
    for (map<GlobalVariable *, GlobalVariable *>::iterator iter = old2new.begin(); iter != old2new.end(); ++iter) {
      GlobalVariable *toDelete = iter->first;
      toDelete->removeDeadConstantUsers();
      if (toDelete->getNumUses() == 0) {
        toDelete->dropAllReferences();
        toDelete->eraseFromParent();
      }
    }
    GlobalVariable *StatusGV = encstatus[Func];
    // Insert DecryptionCode
    HandleDecryptionBlock(B, C, GV2Keys);
    // Add atomic load checking status in A
    LoadInst *LI = IRB.CreateLoad(StatusGV->getType()->getPointerElementType(), StatusGV, "LoadEncryptionStatus");
    LI->setAtomic(AtomicOrdering::Acquire); // Will be released at the start of C
    LI->setAlignment(llvm::Align(4));
    Value *condition = IRB.CreateICmpEQ(LI, ConstantInt::get(Type::getInt32Ty(Func->getContext()), 0));
    A->getTerminator()->eraseFromParent();
    BranchInst::Create(B, C, condition, A);
    // Add StoreInst atomically in C start
    // No matter control flow is coming from A or B, the GVs must be decrypted
    IRBuilder<> IRBC(C->getFirstNonPHIOrDbgOrLifetime());
    StoreInst *SI = IRBC.CreateStore(ConstantInt::get(Type::getInt32Ty(Func->getContext()), 1), StatusGV);
    SI->setAlignment(llvm::Align(4));
    SI->setAtomic(AtomicOrdering::Release); // Release the lock acquired in LI
} // End of HandleFunction

void StringEncryptionPass::HandleDecryptionBlock(BasicBlock *B, BasicBlock *C, map<GlobalVariable *, Constant *> &GV2Keys) {
    IRBuilder<> IRB(B);
    Value *zero = ConstantInt::get(Type::getInt32Ty(B->getContext()), 0);
    for (map<GlobalVariable *, Constant *>::iterator iter = GV2Keys.begin();
            iter != GV2Keys.end(); ++iter) {
        ConstantDataArray *CastedCDA = cast<ConstantDataArray>(iter->second);
        // Element-By-Element XOR so the fucking verifier won't complain
        // Also, this hides keys
        for (unsigned i = 0; i < CastedCDA->getType()->getNumElements(); i++) {
            Value *offset = ConstantInt::get(Type::getInt32Ty(B->getContext()), i);
            Value *GEP = IRB.CreateGEP(iter->first->getType()->getPointerElementType(), iter->first, {zero, offset});
            LoadInst *LI = IRB.CreateLoad(GEP->getType()->getPointerElementType(), GEP, "EncryptedChar");
            Value *XORed = IRB.CreateXor(LI, CastedCDA->getElementAsConstant(i));
            IRB.CreateStore(XORed, GEP);
        }
    }
    IRB.CreateBr(C);
} // End of HandleDecryptionBlock

bool StringEncryptionPass::doFinalization(Module &M){
    encstatus.clear();
    return false;
}

StringEncryptionPass *llvm::createStringEncryption(bool flag){
    return new StringEncryptionPass(flag);
}