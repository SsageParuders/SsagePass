#include "SplitBasicBlock.h" // 基本块分割
#include "Flattening.h"  // 控制流平坦化
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
            PB.registerPipelineParsingCallback(
              [&](StringRef Name, FunctionPassManager &FPM,
                  ArrayRef<PassBuilder::PipelineElement>) {
                if(Name == "obf-fla"){ // 注册虚假控制流
                  FPM.addPass(FlatteningPass(false));
                  return true;
                }
                if(Name == "obf-split"){ // 注册基本块分割
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