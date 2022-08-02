#include "SplitBasicBlock.h" // 基本块分割
#include "Flattening.h"  // 控制流平坦化
#include "StringEncryption.h" // 字符串加密
#include "VMFlatten.h" // 虚拟机控制流平坦化
#include "IndirectBranch.h" // 间接调用
#include "FunctionWrapper.h" // 函数嵌套
#include "BogusControlFlow.h" // 虚假控制流
#include "llvm/Transforms/Utils/SymbolRewriter.h" // 重命名符号
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
            outs() << "Version is " << 20 << "\n";
            // for opt
            /**
             * @brief 暂时适配Android NDK下的llvm环境 由于没有opt 且opt不适合集成编译 暂时不适配这方面参数
             * 
             */
            // PB.registerPipelineParsingCallback(
            //   [&](StringRef Name, FunctionPassManager &FPM,
            //     ArrayRef<PassBuilder::PipelineElement>) {
            //     if(Name == "fla"){ // 注册控制流平坦化
            //       FPM.addPass(FlatteningPass(false));
            //       return true;
            //     }
            //     if(Name == "split"){ // 注册基本块分割
            //       FPM.addPass(SplitBasicBlockPass(false));
            //       return true;
            //     }
            //     if(Name == "bcf"){ // 注册虚假控制流
            //       FPM.addPass(BogusControlFlowPass(false));
            //       return true;
            //     }
            //     return false;
            // });
            // // for opt
            // PB.registerPipelineParsingCallback(
            //     [&](StringRef Name, ModulePassManager &MPM,
            //         ArrayRef<PassBuilder::PipelineElement>){
            //       if (Name == "strenc"){
            //         MPM.addPass(StringEncryptionPass(false));
            //         return true;
            //       }
            //       if (Name == "indibr"){
            //         MPM.addPass(IndirectBranchPass(false));
            //       }
            //       if (Name == "funwra"){
            //         MPM.addPass(IndirectBranchPass(false));
            //       }
            //       return false;
            // });
            // clang
            PB.registerPipelineStartEPCallback(
                [](llvm::ModulePassManager &MPM, // 模块Pass 作用于某个c文件内
                   llvm::OptimizationLevel Level){
                    MPM.addPass(StringEncryptionPass(false)); // 先进行字符串加密 出现字符串加密基本块以后 再进行基本块分割和其他混淆 加大解密难度
                    llvm::FunctionPassManager FPM;
                    FPM.addPass(SplitBasicBlockPass(false));  // 优先进行基本块分割
                    FPM.addPass(VMFlattenPass(false)); // 虚拟机控制流平坦化
                    FPM.addPass(FlatteningPass(false));       // 对于控制流平坦化 不提前开启LowerSwitch 只在控制流平坦化内调用LegacyLowerSwitch
                    FPM.addPass(BogusControlFlowPass(false)); // 虚假控制流
                    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
                    MPM.addPass(FunctionWrapperPass(false)); // 函数包装 理论上函数包装最好也是放在最后
                    MPM.addPass(IndirectBranchPass(false)); // 间接指令 理论上间接指令应该放在最后
                    MPM.addPass(RewriteSymbolPass()); // 根据yaml信息 重命名特定symbols
            });
            // 这里的注册时机不好 弃用以下方案 改用上面的方案
            // 自动注册 需要添加 -O1 参数 然则可能部分pass不生效
            // PB.registerVectorizerStartEPCallback(
            //     [](llvm::FunctionPassManager &FPM, // 函数Pass 作用于某个函数内
            //        llvm::OptimizationLevel Level){
            //         FPM.addPass(SplitBasicBlockPass(false));  // 优先进行基本块分割
            //         FPM.addPass(FlatteningPass(false));       // 对于控制流平坦化 不提前开启LowerSwitch 只在控制流平坦化内调用LegacyLowerSwitch
            //         FPM.addPass(BogusControlFlowPass(false)); // 虚假控制流
            // });
            outs() << "Made By SsageParuders\n";
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