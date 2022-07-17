#include "Utils.h"
#include "Flattening.h"
#include "SplitBasicBlock.h"
#include "LegacyLowerSwitch.h"
// namespace
using namespace llvm;
using std::vector;

#define DEBUG_TYPE "flattening" // 调试标识
// Stats
STATISTIC(Flattened, "Functions flattened");

namespace{
    class Flattening : public FunctionPass {
        public:
            static char ID;
            bool flag;
            Flattening() : FunctionPass(ID){
                srand(time(0));
            }
            Flattening(bool flag) : FunctionPass(ID) { this->flag = flag; }
            // 对函数 F 进行平坦化
            void flatten(Function &F);
            bool runOnFunction(Function &F);
    };
}

bool Flattening::runOnFunction(Function &F){
    Function *tmp = &F;
    // 判断是否需要开启控制流平坦化
    if (toObfuscate(flag, tmp, "fla")) {
        outs() << "\033[44;37m============Flattening Start============\033[0m\n";
        outs() << "\033[42;35mFunction : " << F.getName() << "\033[0m\n"; // 打印一下被混淆函数的symbol
        INIT_CONTEXT(F);
        FunctionPass *pass = createSplitBasicBlock(flag); // 在虚假控制流之前先进行基本块分割 以提高混淆程度
        pass->runOnFunction(F); // 在虚假控制流之前先进行基本块分割 以提高混淆程度
        flatten(*tmp);
        ++Flattened;
        outs() << "\033[44;37m============Flattening Finish============\033[0m\n";
    }
    return true;
}

void Flattening::flatten(Function &F){
    outs() << "\033[42;35mFunction size : " << F.size() << "\033[0m\n";
    // 基本块数量不超过1则无需平坦化
    if(F.size() <= 1){
        outs() << "\033[43;33mFunction size is lower then one\033[0m\n"; // warning
        return;
    }

    // Lower switch
    // 调用 Lower switch 会导致崩溃，解决方法未知
    // FunctionPass *pass = createLowerSwitchPass();
    // pass->runOnFunction(F);

    // Lower switch
    FunctionPass *lower = createLegacyLowerSwitchPass();
    lower->runOnFunction(F);

    outs() << "Lower switch had open\n";

    // 将除入口块（第一个基本块）以外的基本块保存到一个 vector 容器中，便于后续处理
    // 首先保存所有基本块
    vector<BasicBlock*> origBB;
    for(BasicBlock &BB: F){
        origBB.push_back(&BB);
    }
    // 从vector中去除第一个基本块
    origBB.erase(origBB.begin());
    BasicBlock &entryBB = F.getEntryBlock();
    // 如果第一个基本块的末尾是条件跳转，单独分离
    if(BranchInst *br = dyn_cast<BranchInst>(entryBB.getTerminator())){
        if(br->isConditional()){
            BasicBlock *newBB = entryBB.splitBasicBlock(br, "newBB");
            origBB.insert(origBB.begin(), newBB);
        }
    }

    // 创建分发块和返回块
    BasicBlock *dispatchBB = BasicBlock::Create(*CONTEXT, "dispatchBB", &F, &entryBB);
    BasicBlock *returnBB = BasicBlock::Create(*CONTEXT, "returnBB", &F, &entryBB);
    BranchInst::Create(dispatchBB, returnBB);
    entryBB.moveBefore(dispatchBB);
    // 去除第一个基本块末尾的跳转
    entryBB.getTerminator()->eraseFromParent();
    // 使第一个基本块跳转到dispatchBB
    BranchInst *brDispatchBB = BranchInst::Create(dispatchBB, &entryBB);

    // 在入口块插入alloca和store指令创建并初始化switch变量，初始值为随机值
    int randNumCase = rand();
    AllocaInst *swVarPtr = new AllocaInst(TYPE_I32, 0, "swVar.ptr", brDispatchBB);
    new StoreInst(CONST_I32(randNumCase), swVarPtr, brDispatchBB);
    // 在分发块插入load指令读取switch变量
    LoadInst *swVar = new LoadInst(TYPE_I32, swVarPtr, "swVar", false, dispatchBB);
    // 在分发块插入switch指令实现基本块的调度
    BasicBlock *swDefault = BasicBlock::Create(*CONTEXT, "swDefault", &F, returnBB);
    BranchInst::Create(returnBB, swDefault);
    SwitchInst *swInst = SwitchInst::Create(swVar, swDefault, 0, dispatchBB);
    // 将原基本块插入到返回块之前，并分配case值
    for(BasicBlock *BB : origBB){
        BB->moveBefore(returnBB);
        swInst->addCase(CONST_I32(randNumCase), BB);
        randNumCase = rand();
    }

    // 在每个基本块最后添加修改switch变量的指令和跳转到返回块的指令
    for(BasicBlock *BB : origBB){
        // retn BB
        if(BB->getTerminator()->getNumSuccessors() == 0){
            continue;
        }
        // 非条件跳转
        else if(BB->getTerminator()->getNumSuccessors() == 1){
            BasicBlock *sucBB = BB->getTerminator()->getSuccessor(0);
            BB->getTerminator()->eraseFromParent();
            ConstantInt *numCase = swInst->findCaseDest(sucBB);
            new StoreInst(numCase, swVarPtr, BB);
            BranchInst::Create(returnBB, BB);
        }
        // 条件跳转
        else if(BB->getTerminator()->getNumSuccessors() == 2){
            ConstantInt *numCaseTrue = swInst->findCaseDest(BB->getTerminator()->getSuccessor(0));
            ConstantInt *numCaseFalse = swInst->findCaseDest(BB->getTerminator()->getSuccessor(1));
            BranchInst *br = cast<BranchInst>(BB->getTerminator());
            SelectInst *sel = SelectInst::Create(br->getCondition(), numCaseTrue, numCaseFalse, "", BB->getTerminator());
            BB->getTerminator()->eraseFromParent();
            new StoreInst(sel, swVarPtr, BB);
            BranchInst::Create(returnBB, BB);
        }
    }
    fixStack(F); // 修复逃逸变量和PHI指令
}

FunctionPass *llvm::createFlattening(bool flag) {
    return new Flattening(flag);
}

char Flattening::ID = 0;
static RegisterPass<Flattening> X("fla", "Flatten the basic blocks in each function."); // 注册PASS