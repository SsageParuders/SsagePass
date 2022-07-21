#include "SplitBasicBlock.h" // 基本块分割
#include "Flattening.h"  // 控制流平坦化
#include "StringEncryption.h" // 字符串加密
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
            outs() << "Version is " << 4 << "\n";
            PB.registerPipelineParsingCallback( // 优先进行字符串混淆 以方便对字符串加密基本块做分割和混淆
                [&](StringRef Name, ModulePassManager &MPM,
                    ArrayRef<PassBuilder::PipelineElement>){
                  if (Name == "strenc"){ // 注册字符串混淆
                    MPM.addPass(StringEncryptionPass(false));
                    return true;
                  }
                  return false;
                });
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
                  return false;
                });
            // 自动注册 需要添加 -O1 参数 然则可能部分pass不生效
            PB.registerVectorizerStartEPCallback(
              [](llvm::FunctionPassManager &FPM,
                  llvm::OptimizationLevel Level){
                FPM.addPass(SplitBasicBlockPass(false)); // 优先进行基本块分割
                FPM.addPass(FlatteningPass(false)); // 对于控制流平坦化 不提前开启LowerSwitch 只在控制流平坦化内调用LegacyLowerSwitch
            });
            PB.registerOptimizerLastEPCallback(
                [](llvm::ModulePassManager &MPM,
                  llvm::OptimizationLevel Level){
                MPM.addPass(StringEncryptionPass(false)); // 字符串加密
            });
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