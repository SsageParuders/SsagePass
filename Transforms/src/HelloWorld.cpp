// LLVM libs
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
// User libs
#include "Utils.h"
// namespace
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
    // F.getEntryBlock();
    // outs() << "Hello, " << F.getName() << "\n";
    // return true;
    Function *tmp = &F;
    // Do we obfuscate
    if (toObfuscate(false, tmp, "hlw")) {
        outs() << "Hello, " << F.getName() << "\n";
    }

    return false;
}

char Demo::ID = 0;
static RegisterPass<Demo> X("hlw", "This is first pass by SsageParuders");