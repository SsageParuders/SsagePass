#include "SplitBasicBlock.h" // 基本块分割
#include "Flattening.h"  // 控制流平坦化
#include "StringEncryption.h" // 字符串加密
#include "IndirectBranch.h" //
#include "FunctionWrapper.h" //
#include "BogusControlFlow.h" // 虚假控制流
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

/**
 * @brief Get the Ssage Plugin Info object
 * 
 * @return llvm::PassPluginLibraryInfo 
 */
llvm::PassPluginLibraryInfo getSsagePluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "Ssage", LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            outs() << "Version is " << 12 << "\n";
            // for opt
            PB.registerPipelineParsingCallback(
              [&](StringRef Name, FunctionPassManager &FPM,
                ArrayRef<PassBuilder::PipelineElement>) {
                if(Name == "fla"){ // 注册控制流平坦化
                  FPM.addPass(FlatteningPass(false));
                  return true;
                }
                if(Name == "split"){ // 注册基本块分割
                  FPM.addPass(SplitBasicBlockPass(false));
                  return true;
                }
                if(Name == "bcf"){ // 注册虚假控制流
                  FPM.addPass(BogusControlFlowPass(false));
                  return true;
                }
                return false;
            });
            // for opt
            PB.registerPipelineParsingCallback(
                [&](StringRef Name, ModulePassManager &MPM,
                    ArrayRef<PassBuilder::PipelineElement>){
                  if (Name == "strenc"){
                    MPM.addPass(StringEncryptionPass(false));
                    return true;
                  }
                  if (Name == "indibr"){
                    MPM.addPass(IndirectBranchPass(false));
                  }
                  if (Name == "funwra"){
                    MPM.addPass(IndirectBranchPass(false));
                  }
                  return false;
            });
            // clang
            PB.registerPipelineStartEPCallback(
                [](llvm::ModulePassManager &MPM, // 模块Pass 作用于某个c文件内
                   llvm::OptimizationLevel Level){
                    MPM.addPass(StringEncryptionPass(false)); // 先进行字符串加密 出现字符串加密基本块以后 再进行基本块分割和其他混淆 加大解密难度
                    MPM.addPass(FunctionWrapperPass(false)); // 函数包装
                    MPM.addPass(IndirectBranchPass(false)); // 间接指令
            });
            // 自动注册 需要添加 -O1 参数 然则可能部分pass不生效
            PB.registerVectorizerStartEPCallback(
                [](llvm::FunctionPassManager &FPM, // 函数Pass 作用于某个函数内
                   llvm::OptimizationLevel Level){
                    FPM.addPass(SplitBasicBlockPass(false));  // 优先进行基本块分割
                    FPM.addPass(FlatteningPass(false));       // 对于控制流平坦化 不提前开启LowerSwitch 只在控制流平坦化内调用LegacyLowerSwitch
                    FPM.addPass(BogusControlFlowPass(false)); // 虚假控制流
            });
            outs() << "";
        }};
}

/**
 * @brief NEW PM
 * 
 */
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getSsagePluginInfo();
}