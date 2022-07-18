#include "SplitBasicBlock.h" // 基本块分割
// #include "Flattening.h"  // 控制流平坦化
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
            // // 注册标记 "opt -passes=obf-split"
            PB.registerPipelineParsingCallback(
              [&](StringRef Name, FunctionPassManager &FPM,
                  ArrayRef<PassBuilder::PipelineElement>) {
                if(Name == "obf-split"){
                  FPM.addPass(SplitBasicBlockPass(false));
                  return true;
                }
                return false;
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