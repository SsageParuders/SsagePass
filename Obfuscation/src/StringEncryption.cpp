#include "StringEncryption.h"

#define DEBUG_TYPE "strenc"

using namespace llvm;

static cl::opt<int>
    ObfuTimes("gle-times", cl::init(1),
              cl::desc("Run GlobalsEncryption pass <gle-times> time(s)"));
static cl::opt<bool> OnlyStr("onlystr", cl::init(true),
                             cl::desc("Encrypt string variable only"));

PreservedAnalyses StringEncryptionPass::run(Module &M, ModuleAnalysisManager& AM) {
        if (toObfuscate(flag, F, "strenc")){
            for (Module::iterator iter = M.begin(); iter != M.end(); iter++) {
                Function *F = &(*iter);
                outs() << "\033[1;32m[StringEncryption] Function : " << F->getName() << "\033[0m\n"; // 打印一下被混淆函数的symbol
            }
            INIT_CONTEXT(M);
            vector<GlobalVariable *> GVs;
            for (GlobalVariable &GV : M.getGlobalList()) {
                GVs.push_back(&GV);
            }
            for (int i = 0; i < ObfuTimes; i++) {
                for (GlobalVariable *GV : GVs) {
                // Only encrypt globals of integer and array
                if (!GV->getValueType()->isIntegerTy() &&
                    !GV->getValueType()->isArrayTy()) {
                    continue;
                }
                if (GV->hasInitializer() && GV->getInitializer() &&
                    (GV->getName().contains(".str") || !OnlyStr)
                    // Do not encrypt globals having a section named "llvm.metadata"
                    && !GV->getSection().equals("llvm.metadata")
                    && GV->getSection().find(StringRef("__objc")) == string::npos && GV->getName().find("OBJC") == string::npos) {
                    Constant *initializer = GV->getInitializer();
                    ConstantInt *intData = dyn_cast<ConstantInt>(initializer);
                    ConstantDataArray *arrData = dyn_cast<ConstantDataArray>(initializer);

                    if (arrData) {
                    uint32_t eleSize = arrData->getElementByteSize();
                    uint32_t eleNum = arrData->getNumElements();
                    uint32_t arrLen = eleNum * eleSize;
                    char *data = const_cast<char *>(arrData->getRawDataValues().data());
                    char *dataCopy = new char[arrLen];
                    memcpy(dataCopy, data, arrLen);
                    uint64_t key = cryptoutils->get_uint64_t();
                    // A simple xor encryption
                    for (uint32_t i = 0; i < arrLen; i++) {
                        dataCopy[i] ^= ((char *)&key)[i % eleSize];
                    }
                    GV->setInitializer(ConstantDataArray::getRaw(
                        StringRef(dataCopy, arrLen), eleNum, arrData->getElementType()));
                    GV->setConstant(false);
                    insertArrayDecryption(M, {GV, key, eleNum});
                    } else if (intData) {
                    uint64_t key = cryptoutils->get_uint64_t();
                    ConstantInt *enc =
                        CONST(intData->getType(), key ^ intData->getZExtValue());
                    GV->setInitializer(enc);
                    GV->setConstant(false);
                    insertIntDecryption(M, {GV, key, 1LL});
                    }
                }
                }
            }
        }

    return PreservedAnalyses::all();
}
/*
    Generate a hashed name for the decryption function of a global variable to
    be encrypted
    e.g. 44b5bc44f8b7f2d95383c20f3ee762c9beee8913
*/
string genHashedName(GlobalVariable *GV) {
    Module &M = *GV->getParent();
    string funcName =
        formatv("{0}_{1:x-}", M.getName(), M.getMDKindID(GV->getName()));
    SHA1 sha1;
    sha1.init();
    sha1.update(funcName);
    StringRef digest = sha1.final();
    std::stringstream ss;
    ss << std::hex;
    for (size_t i = 0; i < digest.size(); i++) {
        ss << std::setw(2) << std::setfill('0') << (unsigned)(digest[i] & 0xFF);
    }
    return ss.str();
}

void StringEncryptionPass::insertIntDecryption(Module &M, EncryptedGV encGV) {
    vector<Type *> args;
    FunctionType *funcType =
        FunctionType::get(Type::getVoidTy(M.getContext()), args, false);
    string funcName = genHashedName(encGV.GV);
    FunctionCallee callee = M.getOrInsertFunction(funcName, funcType);
    Function *func = cast<Function>(callee.getCallee());

    BasicBlock *entry = BasicBlock::Create(*CONTEXT, "entry", func);
    IRBuilder<> builder(*CONTEXT);
    builder.SetInsertPoint(entry);
    LoadInst *val = builder.CreateLoad(encGV.GV->getType()->getPointerElementType(), encGV.GV, "");
    Value *xorVal =
        builder.CreateXor(val, CONST(encGV.GV->getValueType(), encGV.key));
    builder.CreateStore(xorVal, encGV.GV);
    builder.CreateRetVoid();
    appendToGlobalCtors(M, func, 0);
}

void StringEncryptionPass::insertArrayDecryption(Module &M, EncryptedGV encGV) {
    vector<Type *> args;
    FunctionType *funcType =
        FunctionType::get(Type::getVoidTy(M.getContext()), args, false);
    string funcName = genHashedName(encGV.GV);
    FunctionCallee callee = M.getOrInsertFunction(funcName, funcType);
    Function *func = cast<Function>(callee.getCallee());

    BasicBlock *entry = BasicBlock::Create(*CONTEXT, "entry", func);
    BasicBlock *forCond = BasicBlock::Create(*CONTEXT, "for.cond", func);
    BasicBlock *forBody = BasicBlock::Create(*CONTEXT, "for.body", func);
    BasicBlock *forInc = BasicBlock::Create(*CONTEXT, "for.inc", func);
    BasicBlock *forEnd = BasicBlock::Create(*CONTEXT, "for.inc", func);

    IRBuilder<> builder(*CONTEXT);
    builder.SetInsertPoint(entry);
    AllocaInst *indexPtr = builder.CreateAlloca(TYPE_I32, CONST_I32(1), "i");
    builder.CreateStore(CONST_I32(0), indexPtr);
    builder.CreateBr(forCond);
    builder.SetInsertPoint(forCond);
    LoadInst *index = builder.CreateLoad(TYPE_I32, indexPtr);
    ICmpInst *cond =
        cast<ICmpInst>(builder.CreateICmpSLT(index, CONST_I32(encGV.len)));
    builder.CreateCondBr(cond, forBody, forEnd);
    builder.SetInsertPoint(forBody);
    Value *indexList[2] = {CONST_I32(0), index};
    // IRB.CreateGEP(encGV.GV->getType()->getPointerElementType(), encGV.GV, ArrayRef<Value *>(indexList, 2)
    Value *ele = builder.CreateGEP(encGV.GV->getType()->getPointerElementType(), encGV.GV, ArrayRef<Value *>(indexList, 2));
    ArrayType *arrTy = cast<ArrayType>(encGV.GV->getValueType());
    Type *eleTy = arrTy->getElementType();
    Value *encEle =
        builder.CreateXor(builder.CreateLoad(ele->getType()->getPointerElementType(), ele), CONST(eleTy, encGV.key));
    builder.CreateStore(encEle, ele);
    builder.CreateBr(forInc);
    builder.SetInsertPoint(forInc);
    builder.CreateStore(builder.CreateAdd(index, CONST_I32(1)), indexPtr);
    builder.CreateBr(forCond);

    builder.SetInsertPoint(forEnd);
    builder.CreateRetVoid();
    appendToGlobalCtors(M, func, 0);
}

StringEncryptionPass *llvm::createStringEncryption(bool flag){
    return new StringEncryptionPass(flag);
}