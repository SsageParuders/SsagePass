// User libs
#include "Utils.h"
#include "CryptoUtils.h"
#include "SplitBasicBlock.h"
// namespace
using namespace llvm;
using std::vector;

#define DEBUG_TYPE "split" // 调试标识
// Stats
STATISTIC(Split, "Basicblock splitted"); // 宏定义

// 可选的参数，指定一个基本块会被分裂成几个基本块，默认值为 2
static cl::opt<int> SplitNum("split_num", cl::init(2), cl::desc("Split <split_num> time(s) each BB"));

/**
 * @brief 实现效果 选定被混淆的函数 每一条指令都被分割为一个基本块
 * 
 * @param F 
 * @return true 
 * @return false 
 */
bool SplitBasicBlock::runOnFunction(Function &F){
    Function *tmp = &F; // 传入的Function
    if (toObfuscate(flag, tmp, "split")){ // 判断什么函数需要开启混淆
        // \033[42;35mFinish Android Native Build SUCCEED !!\033[0m\n
        // \033[41;37mError !! Build failed !!\033[0m\n
        outs() << "\033[44;37m============SplitBasicBlock Start============\033[0m\n";
        outs() << "\033[42;35mFunction is " << F.getName() << "\033[0m\n"; // 打印一下被混淆函数的symbol
        split(tmp); // 分割流程
        ++Split; // 计次
        outs() << "\033[44;37m============SplitBasicBlock Finish============\033[0m\n";
    }
    return false;
}

/**
 * @brief 对传入的基本块做分割
 * 
 * @param BB 
 */
void SplitBasicBlock::split(Function *f){
    std::vector<BasicBlock *> origBB;
    // 保存所有基本块 防止分割的同时迭代新的基本块
    for (Function::iterator I = f->begin(), IE = f->end(); I != IE; ++I){
        origBB.push_back(&*I);
    }
    // 遍历函数的全部基本块
    for (std::vector<BasicBlock *>::iterator I = origBB.begin(), IE = origBB.end();I != IE; ++I){
        BasicBlock *curr = *I;
        outs() << "\033[42;35mSplitNum is " << SplitNum << "\033[0m\n";
        outs() << "\033[42;35mBasicBlock Size is " << curr->size() << "\033[0m\n";
        int splitN = SplitNum;
        // 无需分割只有一条指令的基本块
        // 不可分割含有PHI指令基本块
        if (curr->size() < 2 || containsPHI(curr)){
            continue;
        }
        // 检查splitN和基本块大小 如果传入的分割块数甚至大于等于基本块自身大小 则修改分割数为基本块大小减一
        if ((size_t)splitN >= curr->size()){
            splitN = curr->size() - 1;
        }
        outs() << "\033[42;35msplitNum Now is " << splitN << "\033[0m\n";
        // Generate splits point
        std::vector<int> test;
        for (unsigned i = 1; i < curr->size(); ++i){
            test.push_back(i);
        }
        // Shuffle
        if (test.size() != 1){
            shuffle(test);
            std::sort(test.begin(), test.begin() + splitN);
        }
        // 分割
        BasicBlock::iterator it = curr->begin();
        BasicBlock *toSplit = curr;
        int last = 0;
        for (int i = 0; i < splitN; ++i){
            if (toSplit->size() < 2)
                continue;
            for (int j = 0; j < test[i] - last; ++j){
                ++it;
            }
            last = test[i];
            toSplit = toSplit->splitBasicBlock(it, toSplit->getName() + ".split");
        }
        ++Split;
    }
}

/**
 * @brief 判断基本块是否包含PHI指令
 * 
 * @param BB 
 * @return true 
 * @return false 
 */
bool SplitBasicBlock::containsPHI(BasicBlock *BB){
    for (Instruction &I : *BB){
        if (isa<PHINode>(&I)){
            return true;
        }
    }
    return false;
}

void SplitBasicBlock::shuffle(std::vector<int> &vec){
    int n = vec.size();
    for (int i = n - 1; i > 0; --i)
    {
        std::swap(vec[i], vec[cryptoutils->get_uint32_t() % (i + 1)]);
    }
}

/**
 * @brief 便于调用基本块分割
 *
 * @param flag
 * @return FunctionPass*
 */
FunctionPass *llvm::createSplitBasicBlock(bool flag){
    return new SplitBasicBlock(flag);
}

char SplitBasicBlock::ID = 0;
// 注册Pass --> 传统注册方案 // TODO: 改变为新的PASS管理器
static RegisterPass<SplitBasicBlock> X("split", "Split a basic block into multiple basic blocks.");