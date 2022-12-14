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
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/SHA1.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

// User libs
#include "CryptoUtils.h"
#include "Utils.h"
// System libs
#include <map>
#include <set>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <vector>

using namespace std;
namespace llvm {
    struct EncryptedGV {
        GlobalVariable *GV;
        uint64_t key;
        uint32_t len;
    };

    class StringEncryptionPass : public PassInfoMixin<StringEncryptionPass>{
        public:
            bool flag;
            map<Function * /*Function*/, GlobalVariable * /*Decryption Status*/>encstatus;
            StringEncryptionPass(bool flag){
                this->flag = flag;
            }
            PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM); // Pass实现函数
            void insertArrayDecryption(Module &M, EncryptedGV encGV);
            void insertIntDecryption(Module &M, EncryptedGV encGV);
            static bool isRequired() { return true; } // 直接返回true即可
    };
    StringEncryptionPass *createStringEncryption(bool flag); // 创建字符串加密
}
#endif