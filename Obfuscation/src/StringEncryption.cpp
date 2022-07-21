// #include "llvm/Transforms/Obfuscation/IPObfuscationContext.h"

#include "StringEncryption.h"

#define DEBUG_TYPE "string-encryption"

using namespace llvm;

PreservedAnalyses StringEncryptionPass::run(Module &M, ModuleAnalysisManager& AM) {
    std::set<GlobalVariable *> ConstantStringUsers;

    // 收集全部c/c++字符串
    LLVMContext &Ctx = M.getContext();
    ConstantInt *Zero = ConstantInt::get(Type::getInt32Ty(Ctx), 0);
    for (GlobalVariable &GV : M.globals()){
        if (!GV.isConstant() || !GV.hasInitializer()){
            continue;
        }
        Constant *Init = GV.getInitializer();
        if (Init == nullptr)
            continue;
        if (ConstantDataSequential *CDS = dyn_cast<ConstantDataSequential>(Init)){
            if (CDS->isCString()){
                CSPEntry *Entry = new CSPEntry();
                StringRef Data = CDS->getRawDataValues();
                Entry->Data.reserve(Data.size());
                for (unsigned i = 0; i < Data.size(); ++i){
                    Entry->Data.push_back(static_cast<uint8_t>(Data[i]));
                }
                Entry->ID = static_cast<unsigned>(ConstantStringPool.size());
                ConstantAggregateZero *ZeroInit = ConstantAggregateZero::get(CDS->getType());
                GlobalVariable *DecGV = new GlobalVariable(M, CDS->getType(), false, GlobalValue::PrivateLinkage,
                                                           ZeroInit, "dec" + Twine::utohexstr(Entry->ID) + GV.getName());
                GlobalVariable *DecStatus = new GlobalVariable(M, Type::getInt32Ty(Ctx), false, GlobalValue::PrivateLinkage,
                                                               Zero, "dec_status_" + Twine::utohexstr(Entry->ID) + GV.getName());
                // DecGV->setAlignment(GV.getAlignment());
                Entry->DecGV = DecGV;
                Entry->DecStatus = DecStatus;
                ConstantStringPool.push_back(Entry);
                CSPEntryMap[&GV] = Entry;
                collectConstantStringUser(&GV, ConstantStringUsers);
            }
        }
    }

    // encrypt those strings, build corresponding decrypt function
    // 加密这些字符串 构建相应的解密函数
    for (CSPEntry *Entry: ConstantStringPool) {
        getRandomBytes(Entry->EncKey, 16, 32);
        for (unsigned i = 0; i < Entry->Data.size(); ++i) {
        Entry->Data[i] ^= Entry->EncKey[i % Entry->EncKey.size()];
        }
        Entry->DecFunc = buildDecryptFunction(&M, Entry);
    }

    // build initialization function for supported constant string users
    // 为支持的常量字符串用户构建初始化函数
    for (GlobalVariable *GV: ConstantStringUsers) {
        if (isValidToEncrypt(GV)) {
            Type *EltType = GV->getType()->getElementType();
            ConstantAggregateZero *ZeroInit = ConstantAggregateZero::get(EltType);
            GlobalVariable *DecGV = new GlobalVariable(M, EltType, false, GlobalValue::PrivateLinkage,
                                                        ZeroInit, "dec_" + GV->getName());
            // DecGV->setAlignment(GV->getAlignment());
            GlobalVariable *DecStatus = new GlobalVariable(M, Type::getInt32Ty(Ctx), false, GlobalValue::PrivateLinkage, Zero, "dec_status_" + GV->getName());
            CSUser *User = new CSUser(GV, DecGV);
            User->DecStatus = DecStatus;
            User->InitFunc = buildInitFunction(&M, User);
            CSUserMap[GV] = User;
        }
    }

    // emit the constant string pool
    // | junk bytes | key 1 | encrypted string 1 | junk bytes | key 2 | encrypted string 2 | ...
    std::vector<uint8_t> Data;
    std::vector<uint8_t> JunkBytes;

    JunkBytes.reserve(32);
    for (CSPEntry *Entry: ConstantStringPool) {
        JunkBytes.clear();
        getRandomBytes(JunkBytes, 16, 32);
        Data.insert(Data.end(), JunkBytes.begin(), JunkBytes.end());
        Entry->Offset = static_cast<unsigned>(Data.size());
        Data.insert(Data.end(), Entry->EncKey.begin(), Entry->EncKey.end());
        Data.insert(Data.end(), Entry->Data.begin(), Entry->Data.end());
    }

    Constant *CDA = ConstantDataArray::get(M.getContext(), ArrayRef<uint8_t>(Data));
    EncryptedStringTable = new GlobalVariable(M, CDA->getType(), true, GlobalValue::PrivateLinkage, CDA, "EncryptedStringTable");

    // decrypt string back at every use, change the plain string use to the decrypted one
    bool Changed = false;
    for (Function &F:M) {
        if (F.isDeclaration())
        continue;
        Changed |= processConstantStringUse(&F);
    }

    for (auto &I : CSUserMap) {
        CSUser *User = I.second;
        Changed |= processConstantStringUse(User->InitFunc);
    }

    // delete unused global variables
    deleteUnusedGlobalVariable();
    for (CSPEntry *Entry: ConstantStringPool) {
        if (Entry->DecFunc->use_empty()) {
            Entry->DecFunc->eraseFromParent();
        }
    }
    // return Changed;

    //   StringObfuscation sop;
    //   if (sop.runCustomModule(M))
    //         return PreservedAnalyses::none();
    return PreservedAnalyses::all();
}


void StringEncryptionPass::collectConstantStringUser(GlobalVariable *CString, std::set<GlobalVariable *> &Users) {
    SmallPtrSet<Value *, 16> Visited;
    SmallVector<Value *, 16> ToVisit;
    ToVisit.push_back(CString);
    while (!ToVisit.empty()) {
        Value *V = ToVisit.pop_back_val();
        if (Visited.count(V) > 0)
        continue;
        Visited.insert(V);
        for (Value *User:V->users()) {
            if (auto *GV = dyn_cast<GlobalVariable>(User)) {
                Users.insert(GV);
            } else {
                ToVisit.push_back(User);
            }
        }
    }
}

void StringEncryptionPass::getRandomBytes(std::vector<uint8_t> &Bytes, uint32_t MinSize, uint32_t MaxSize) {
    uint32_t N = RandomEngine.get_uint32_t();
    uint32_t Len;
    assert(MaxSize >= MinSize);
    if (MinSize == MaxSize) {
        Len = MinSize;
    } else {
        Len = MinSize + (N % (MaxSize - MinSize));
    }
    char *Buffer = new char[Len];
    RandomEngine.get_bytes(Buffer, Len);
    for (uint32_t i = 0; i < Len; ++i) {
        Bytes.push_back(static_cast<uint8_t>(Buffer[i]));
    }
    delete[] Buffer;
}

Function *StringEncryptionPass::buildDecryptFunction(Module *M, const StringEncryptionPass::CSPEntry *Entry) {
    LLVMContext &Ctx = M->getContext();
    IRBuilder<> IRB(Ctx);
    FunctionType *FuncTy = FunctionType::get(Type::getVoidTy(Ctx), {IRB.getInt8PtrTy(), IRB.getInt8PtrTy()}, false);
    Function *DecFunc =
        Function::Create(FuncTy, GlobalValue::PrivateLinkage, "goron_decrypt_string_" + Twine::utohexstr(Entry->ID), M);

    auto ArgIt = DecFunc->arg_begin();
    Argument *PlainString = ArgIt; // output
    ++ArgIt;
    Argument *Data = ArgIt;       // input

    PlainString->setName("plain_string");
    PlainString->addAttr(Attribute::NoCapture);
    Data->setName("data");
    Data->addAttr(Attribute::NoCapture);
    Data->addAttr(Attribute::ReadOnly);

    BasicBlock *Enter = BasicBlock::Create(Ctx, "Enter", DecFunc);
    BasicBlock *LoopBody = BasicBlock::Create(Ctx, "LoopBody", DecFunc);
    BasicBlock *UpdateDecStatus = BasicBlock::Create(Ctx, "UpdateDecStatus", DecFunc);
    BasicBlock *Exit = BasicBlock::Create(Ctx, "Exit", DecFunc);

    IRB.SetInsertPoint(Enter);
    ConstantInt *KeySize = ConstantInt::get(Type::getInt32Ty(Ctx), Entry->EncKey.size());
    // Value *EncPtr = IRB.CreateInBoundsGEP(Data, KeySize);
    // Value *DecStatus = IRB.CreateLoad(Entry->DecStatus);
    // Value *IsDecrypted = IRB.CreateICmpEQ(DecStatus, IRB.getInt32(1));
    // IRB.CreateCondBr(IsDecrypted, Exit, LoopBody);

    IRB.SetInsertPoint(LoopBody);
    PHINode *LoopCounter = IRB.CreatePHI(IRB.getInt32Ty(), 2);
    LoopCounter->addIncoming(IRB.getInt32(0), Enter);

    // Value *EncCharPtr = IRB.CreateInBoundsGEP(EncPtr, LoopCounter);
    // Value *EncChar = IRB.CreateLoad(EncCharPtr);
    Value *KeyIdx = IRB.CreateURem(LoopCounter, KeySize);

    // Value *KeyCharPtr = IRB.CreateInBoundsGEP(Data, KeyIdx);
    // Value *KeyChar = IRB.CreateLoad(KeyCharPtr);

    // Value *DecChar = IRB.CreateXor(EncChar, KeyChar);
    // Value *DecCharPtr = IRB.CreateInBoundsGEP(PlainString, LoopCounter);
    // IRB.CreateStore(DecChar, DecCharPtr);

    // Value *NewCounter = IRB.CreateAdd(LoopCounter, IRB.getInt32(1), "", true, true);
    // LoopCounter->addIncoming(NewCounter, LoopBody);

    // Value *Cond = IRB.CreateICmpEQ(NewCounter, IRB.getInt32(static_cast<uint32_t>(Entry->Data.size())));
    // IRB.CreateCondBr(Cond, UpdateDecStatus, LoopBody);

    // IRB.SetInsertPoint(UpdateDecStatus);
    // IRB.CreateStore(IRB.getInt32(1), Entry->DecStatus);
    // IRB.CreateBr(Exit);

    // IRB.SetInsertPoint(Exit);
    // IRB.CreateRetVoid();

    return DecFunc;
}

bool StringEncryptionPass::isValidToEncrypt(GlobalVariable *GV) {
    if(GV->isConstant() && GV->hasInitializer()) {
        return GV->getInitializer() != nullptr;
    } else {
        return false;
    }
}

Function *StringEncryptionPass::buildInitFunction(Module *M, const StringEncryptionPass::CSUser *User){
    LLVMContext &Ctx = M->getContext();
    IRBuilder<> IRB(Ctx);
    FunctionType *FuncTy = FunctionType::get(Type::getVoidTy(Ctx), {User->DecGV->getType()}, false);
    Function *InitFunc = Function::Create(FuncTy, GlobalValue::PrivateLinkage, "__global_variable_initializer_" + User->GV->getName(), M);

    // auto ArgIt = InitFunc->arg_begin();
    // Argument *thiz = ArgIt;

    // thiz->setName("this");
    // thiz->addAttr(Attribute::NoCapture);

    // convert constant initializer into a series of instructions
    // BasicBlock *Enter = BasicBlock::Create(Ctx, "Enter", InitFunc);
    // BasicBlock *InitBlock = BasicBlock::Create(Ctx, "InitBlock", InitFunc);
    // BasicBlock *Exit = BasicBlock::Create(Ctx, "Exit", InitFunc);

    // IRB.SetInsertPoint(Enter);
    // Value *DecStatus = IRB.CreateLoad(User->DecStatus);
    // Value *IsDecrypted = IRB.CreateICmpEQ(DecStatus, IRB.getInt32(1));
    // IRB.CreateCondBr(IsDecrypted, Exit, InitBlock);

    // IRB.SetInsertPoint(InitBlock);
    // Constant *Init = User->GV->getInitializer();
    // lowerGlobalConstant(Init, IRB, User->DecGV);
    // IRB.CreateStore(IRB.getInt32(1), User->DecStatus);
    // IRB.CreateBr(Exit);

    // IRB.SetInsertPoint(Exit);
    // IRB.CreateRetVoid();
    return InitFunc;
}

void StringEncryptionPass::deleteUnusedGlobalVariable() {
    bool Changed = true;
    while (Changed) {
        Changed = false;
        for (auto Iter = MaybeDeadGlobalVars.begin(); Iter != MaybeDeadGlobalVars.end();) {
            GlobalVariable *GV = *Iter;
            if (!GV->hasLocalLinkage()) {
                ++Iter;
                continue;
            }
            GV->removeDeadConstantUsers();
            if (GV->use_empty()) {
                if (GV->hasInitializer()) {
                    Constant *Init = GV->getInitializer();
                    GV->setInitializer(nullptr);
                    if (isSafeToDestroyConstant(Init))
                        Init->destroyConstant();
                }
                Iter = MaybeDeadGlobalVars.erase(Iter);
                GV->eraseFromParent();
                Changed = true;
            } else {
                ++Iter;
            }
        }
    }
}


bool StringEncryptionPass::processConstantStringUse(Function *F) {
    if (!toObfuscate(false, F, "cse")) { // 对函数的过滤
        return false;
    }
//   if (Options && Options->skipFunction(F->getName())) {
//     return false;
//   }
//   LowerConstantExpr(*F);
//   SmallPtrSet<GlobalVariable *, 16> DecryptedGV; // if GV has multiple use in a block, decrypt only at the first use
//   bool Changed = false;
//   for (BasicBlock &BB : *F) {
//     DecryptedGV.clear();
//     for (Instruction &Inst: BB) {
//       if (PHINode *PHI = dyn_cast<PHINode>(&Inst)) {
//         for (unsigned int i = 0; i < PHI->getNumIncomingValues(); ++i) {
//           if (GlobalVariable *GV = dyn_cast<GlobalVariable>(PHI->getIncomingValue(i))) {
//             auto Iter1 = CSPEntryMap.find(GV);
//             auto Iter2 = CSUserMap.find(GV);
//             if (Iter2 != CSUserMap.end()) { // GV is a constant string user
//               CSUser *User = Iter2->second;
//               if (DecryptedGV.count(GV) > 0) {
//                 Inst.replaceUsesOfWith(GV, User->DecGV);
//               } else {
//                 Instruction *InsertPoint = PHI->getIncomingBlock(i)->getTerminator();
//                 IRBuilder<> IRB(InsertPoint);
//                 IRB.CreateCall(User->InitFunc, {User->DecGV});
//                 Inst.replaceUsesOfWith(GV, User->DecGV);
//                 MaybeDeadGlobalVars.insert(GV);
//                 DecryptedGV.insert(GV);
//                 Changed = true;
//               }
//             } else if (Iter1 != CSPEntryMap.end()) { // GV is a constant string
//               CSPEntry *Entry = Iter1->second;
//               if (DecryptedGV.count(GV) > 0) {
//                 Inst.replaceUsesOfWith(GV, Entry->DecGV);
//               } else {
//                 Instruction *InsertPoint = PHI->getIncomingBlock(i)->getTerminator();
//                 IRBuilder<> IRB(InsertPoint);

//                 Value *OutBuf = IRB.CreateBitCast(Entry->DecGV, IRB.getInt8PtrTy());
//                 Value *Data = IRB.CreateInBoundsGEP(EncryptedStringTable, {IRB.getInt32(0), IRB.getInt32(Entry->Offset)});
//                 IRB.CreateCall(Entry->DecFunc, {OutBuf, Data});

//                 Inst.replaceUsesOfWith(GV, Entry->DecGV);
//                 MaybeDeadGlobalVars.insert(GV);
//                 DecryptedGV.insert(GV);
//                 Changed = true;
//               }
//             }
//           }
//         }
//       } else {
//         for (User::op_iterator op = Inst.op_begin(); op != Inst.op_end(); ++op) {
//           if (GlobalVariable *GV = dyn_cast<GlobalVariable>(*op)) {
//             auto Iter1 = CSPEntryMap.find(GV);
//             auto Iter2 = CSUserMap.find(GV);
//             if (Iter2 != CSUserMap.end()) {
//               CSUser *User = Iter2->second;
//               if (DecryptedGV.count(GV) > 0) {
//                 Inst.replaceUsesOfWith(GV, User->DecGV);
//               } else {
//                 IRBuilder<> IRB(&Inst);
//                 IRB.CreateCall(User->InitFunc, {User->DecGV});
//                 Inst.replaceUsesOfWith(GV, User->DecGV);
//                 MaybeDeadGlobalVars.insert(GV);
//                 DecryptedGV.insert(GV);
//                 Changed = true;
//               }
//             } else if (Iter1 != CSPEntryMap.end()) {
//               CSPEntry *Entry = Iter1->second;
//               if (DecryptedGV.count(GV) > 0) {
//                 Inst.replaceUsesOfWith(GV, Entry->DecGV);
//               } else {
//                 IRBuilder<> IRB(&Inst);

//                 Value *OutBuf = IRB.CreateBitCast(Entry->DecGV, IRB.getInt8PtrTy());
//                 Value *Data = IRB.CreateInBoundsGEP(EncryptedStringTable, {IRB.getInt32(0), IRB.getInt32(Entry->Offset)});
//                 IRB.CreateCall(Entry->DecFunc, {OutBuf, Data});

//                 Inst.replaceUsesOfWith(GV, Entry->DecGV);
//                 MaybeDeadGlobalVars.insert(GV);
//                 DecryptedGV.insert(GV);
//                 Changed = true;
//               }
//             }
//           }
//         }
//       }
//     }
//   }
  return true;
}
