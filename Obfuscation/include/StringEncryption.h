#ifndef LLVM_STRING_ENCRYPTION_H
#define LLVM_STRING_ENCRYPTION_H
// LLVM libs
#include "llvm/Transforms/Utils/GlobalStatus.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
// User libs
#include "CryptoUtils.h"
#include "ObfuscationOptions.h"
#include "Utils.h"
// System libs
#include <map>
#include <set>
#include <iostream>
#include <algorithm>

namespace llvm {
    class StringEncryptionPass : public PassInfoMixin<StringEncryptionPass>{
        public:
            struct CSPEntry {
                CSPEntry() : ID(0), Offset(0), DecGV(nullptr), DecStatus(nullptr), DecFunc(nullptr) {}
                unsigned ID;
                unsigned Offset;
                GlobalVariable *DecGV;
                GlobalVariable *DecStatus; // is decrypted or not
                std::vector<uint8_t> Data;
                std::vector<uint8_t> EncKey;
                Function *DecFunc;
            };
            struct CSUser{
                CSUser(GlobalVariable *User, GlobalVariable *NewGV) : GV(User), DecGV(NewGV), DecStatus(nullptr), InitFunc(nullptr) {}
                GlobalVariable *GV;
                GlobalVariable *DecGV;
                GlobalVariable *DecStatus; // is decrypted or not
                Function *InitFunc;        // InitFunc will use decryted string to initialize DecGV
            };
            ObfuscationOptions *Options;
            CryptoUtils RandomEngine;
            std::vector<CSPEntry *> ConstantStringPool;
            std::map<GlobalVariable *, CSPEntry *> CSPEntryMap;
            std::map<GlobalVariable *, CSUser *> CSUserMap;
            GlobalVariable *EncryptedStringTable;
            std::set<GlobalVariable *> MaybeDeadGlobalVars;
            PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM); // Pass实现函数
            void collectConstantStringUser(GlobalVariable *CString, std::set<GlobalVariable *> &Users); // 收集字符串
            void getRandomBytes(std::vector<uint8_t> &Bytes, uint32_t MinSize, uint32_t MaxSize); //
            Function *buildDecryptFunction(Module *M, const CSPEntry *Entry); //
            Function *buildInitFunction(Module *M, const CSUser *User);
            bool isValidToEncrypt(GlobalVariable *GV); //
            void deleteUnusedGlobalVariable();
            bool processConstantStringUse(Function *F); // 
            static bool isRequired() { return true; } // 直接返回true即可
    };
    StringEncryptionPass *createStringEncryption(bool flag); // 创建字符串加密
}
#endif