#ifndef LLVM_MBAOBFUSCATION_H
#define LLVM_MBAOBFUSCATION_H

// LLVM libs
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

// System libs
#include <vector>
#include <cstdlib>
#include <ctime>

#define RANDOM(X) (cryptoutils->get_uint8_t() % 100 < X)

namespace llvm{
    class MBAObfuscation : public PassInfoMixin<MBAObfuscation>{ 
        public:
            bool flag;
            MBAObfuscation(bool flag){
                this->flag = flag;
            } // 携带flag的构造函数
            PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
            void DoMBAObfuscation(Function &F);
            static bool isRequired() { return true; }
    void substituteConstant(Instruction *I, int i);

    void substitute(BinaryOperator *BI);
    // 替换 Add 指令
    Value *substituteAdd(BinaryOperator *BI);

    // 替换 Sub 指令
    Value *substituteSub(BinaryOperator *BI);

    // 替换 And 指令
    Value *substituteAnd(BinaryOperator *BI);

    // 替换 Or 指令
    Value *substituteOr(BinaryOperator *BI);

    // 替换 Xor 指令
    Value *substituteXor(BinaryOperator *BI);            
    };
    MBAObfuscation *createMBA(bool flag);
}
#endif // LLVM_MBAOBFUSCATION_H