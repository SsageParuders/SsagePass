#ifndef _FUNCTION_WRAPPER_H_
#define _FUNCTION_WRAPPER_H_
// LLVM libs
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Passes/PassBuilder.h"
// System libs
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
// User libs
#include "compat/CallSite.h"
#include "CryptoUtils.h"
#include "Utils.h"
using namespace std;
using namespace llvm;
namespace llvm{ // 间接跳转
    class FunctionWrapperPass : public PassInfoMixin<FunctionWrapperPass>{
        public:
            bool flag;
            bool initialized;
            FunctionWrapperPass(bool flag){
                this->flag = flag;
            } // 携带flag的构造函数
            PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM); // Pass实现函数
            CallSite* HandleCallSite(CallSite *CS); //
            static bool isRequired() { return true; } // 直接返回true即可
    };
    FunctionWrapperPass *createFunctionWrapper(bool flag); // 创建间接跳转
}
#endif