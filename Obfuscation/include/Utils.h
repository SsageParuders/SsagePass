#ifndef LLVM_UTILS_H
#define LLVM_UTILS_H
// LLVM libs
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Transforms/Utils/Cloning.h"
// System libs
#include <vector>
// 常用宏定义
#define INIT_CONTEXT(F) CONTEXT=&F.getContext()
#define TYPE_I32 Type::getInt32Ty(*CONTEXT)
#define CONST_I32(V) ConstantInt::get(TYPE_I32, V, false)
#define CONST(T, V) ConstantInt::get(T, V)

extern llvm::LLVMContext *CONTEXT;

namespace llvm{
    std::string readAnnotate(Function *f); // 读取llvm.global.annotations中的annotation值
    bool toObfuscate(bool flag, llvm::Function *f, std::string const &attribute); // 判断是否开启混淆
    void fixStack(Function &F); // 修复PHI指令和逃逸变量
}

#endif // LLVM_UTILS_H