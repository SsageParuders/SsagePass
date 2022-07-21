#ifndef LLVM_STRING_ENCRYPTION_H
#define LLVM_STRING_ENCRYPTION_H
// LLVM libs
#include "llvm/Transforms/Utils/GlobalStatus.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
// User libs
#include "CryptoUtils.h"
#include "Utils.h"
// System libs
#include <map>
#include <set>
#include <iostream>
#include <algorithm>
using namespace std;
namespace llvm {
    class StringEncryptionPass : public PassInfoMixin<StringEncryptionPass>{
        public:
            bool flag;
            map<Function * /*Function*/, GlobalVariable * /*Decryption Status*/>encstatus;
            StringEncryptionPass(bool flag){
                this->flag = flag;
            }
            PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM); // Pass实现函数
            void HandleFunction(Function *Func); // 处理字符串加密
            void HandleDecryptionBlock(BasicBlock *B, BasicBlock *C, map<GlobalVariable *, Constant *> &GV2Keys); // 处理解密函数
            bool doFinalization(Module &M);
            static bool isRequired() { return true; } // 直接返回true即可
    };
    StringEncryptionPass *createStringEncryption(bool flag); // 创建字符串加密
}
#endif