#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace {
    class Demo : public FunctionPass{
        public:
            static char ID;
            Demo() : FunctionPass(ID){}
            bool runOnFunction(Function &F);
    };
}

bool Demo::runOnFunction(Function &F){
    outs() << "Hello, " << F.getName() << "\n";
    return true;
}

char Demo::ID = 0;
static RegisterPass<Demo> X("hlw", "This is first pass by SsageParuders");
// static void registerSkeletonPass(const PassManagerBuilder &,
//                                  legacy::PassManagerBase &PM)
// {
//     PM.add(new Demo());
// }
// static RegisterStandardPasses
//     RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
//                    registerSkeletonPass);

// static RegisterStandardPasses RegisterMyPass();
// extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
// llvmGetPassPluginInfo(){
//     // return getOllvmPluginInfo();
// }