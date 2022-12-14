#include "VMFlatten.h"

PreservedAnalyses VMFlattenPass::run(Function& F, FunctionAnalysisManager& AM) {
    Function *tmp = &F; // 传入的Function
    // 判断是否需要开启控制流平坦化
    if (toObfuscate(flag, tmp, "vmf")) {
        outs() << "\033[1;32m[VMFlattening] Function : " << F.getName() << "\033[0m\n"; // 打印一下被混淆函数的symbol
        DoFlatten(tmp,0);
    }
    return PreservedAnalyses::none();
}

std::vector<BasicBlock *> *VMFlattenPass::getBlocks(Function *function, std::vector<BasicBlock *> *lists){
    lists->clear();
    for (BasicBlock &basicBlock : *function){
        lists->push_back(&basicBlock);
    }
    return lists;
}

unsigned int VMFlattenPass::getUniqueNumber(std::vector<unsigned int> *rand_list){
    unsigned int num = rand();
    while (true){
        bool state = true;
        for (std::vector<unsigned int>::iterator n = rand_list->begin(); n != rand_list->end(); n++){
            if (*n == num){
                state = false;
                break;
            }
        }
        if (state){
            break;
        }
        num = rand();
    }
    return num;
}

bool VMFlattenPass::valueEscapes(Instruction *Inst){
    const BasicBlock *BB = Inst->getParent();
    for (const User *U : Inst->users())
    {
        const Instruction *UI = cast<Instruction>(U);
        if (UI->getParent() != BB || isa<PHINode>(UI))
            return true;
    }
    return false;
}

Node *VMFlattenPass::newNode(unsigned int value){
    Node *node = (Node *)malloc(sizeof(Node));
    node->value = value;
    node->bb1 = node->bb2 = NULL;
    return node;
}

VMInst *VMFlattenPass::newInst(unsigned int type, unsigned int op1, unsigned int op2){
    VMInst *code = (VMInst *)malloc(sizeof(VMInst));
    code->type = type;
    code->op1 = op1;
    code->op2 = op2;
    return code;
}

void VMFlattenPass::create_node_inst(std::vector<VMInst *> *all_inst, std::map<Node *, unsigned int> *inst_map, Node *node){
    VMInst *code = newInst(RUN_BLOCK, node->value, 0);
    all_inst->push_back(code);
    inst_map->insert(std::map<Node *, unsigned int>::value_type(node, all_inst->size() - 1));
}

Node *VMFlattenPass::findBBNode(BasicBlock *bb, std::vector<Node *> *all_node){
    for (std::vector<Node *>::iterator i = all_node->begin(); i != all_node->end(); i++){
        if (bb == (*i)->data){
            return *i;
        }
    }
    return NULL;
}

void VMFlattenPass::gen_inst(std::vector<VMInst *> *all_inst, std::map<Node *, unsigned int> *inst_map, Node *node){
    // assert(!(node->bb1==NULL && node->bb2!=NULL));
    if (node->bb1 != NULL && node->bb2 == NULL){
        if (inst_map->count(node->bb1) == 0){
            create_node_inst(all_inst, inst_map, node->bb1);
            gen_inst(all_inst, inst_map, node->bb1);
        }else{
            unsigned int addr = (*inst_map->find(node->bb1)).second * 3;
            VMInst *code = newInst(JMP_BORING, addr, 0);
            all_inst->push_back(code);
        }
    } else if (node->bb2 != NULL){
        VMInst *code = newInst(JMP_SELECT, 0, 0);
        all_inst->push_back(code);
        if (inst_map->count(node->bb1) == 0){
            create_node_inst(all_inst, inst_map, node->bb1);
            gen_inst(all_inst, inst_map, node->bb1);
        }
        if (inst_map->count(node->bb2) == 0){
            create_node_inst(all_inst, inst_map, node->bb2);
            gen_inst(all_inst, inst_map, node->bb2);
        }
        code->op1 = (*inst_map->find(node->bb1)).second * 3;
        code->op2 = (*inst_map->find(node->bb2)).second * 3;
    } else {
        return;
    }
}

void VMFlattenPass::dump_inst(std::vector<VMInst *> *all_inst){
    unsigned int x = 0;
    for (std::vector<VMInst *>::iterator i = all_inst->begin(); i != all_inst->end(); i++){
        //printf("\033[1;32m0x%02x: \033[0m", x++);
        VMInst *c = *i;
        if (c->type == RUN_BLOCK){
            //printf("\033[1;32mRUN_BLOCK 0x%02x\033[0m\n", c->op1);
        }
        if (c->type == JMP_BORING){
            //printf("\033[1;32mJMP_BORING 0x%02x\033[0m\n", c->op1);
        }
        if (c->type == JMP_SELECT){
            //printf("\033[1;32mJMP_SELECT 0x%02x 0x%02x\033[0m\n", c->op1, c->op2);
        }
    }
}

void VMFlattenPass::DoFlatten(Function *f, int seed){
    srand(seed);
    std::vector<BasicBlock *> origBB;
    getBlocks(f, &origBB);
    if (origBB.size() <= 1){
        return;
    }
    unsigned int rand_val = seed;
    Function::iterator tmp = f->begin();
    BasicBlock *oldEntry = &*tmp;
    origBB.erase(origBB.begin());
    BranchInst *firstBr = NULL;
    if (isa<BranchInst>(oldEntry->getTerminator())){
        firstBr = cast<BranchInst>(oldEntry->getTerminator());
    }
    BasicBlock *firstbb = oldEntry->getTerminator()->getSuccessor(0);
    if ((firstBr != NULL && firstBr->isConditional()) || oldEntry->getTerminator()->getNumSuccessors() > 2){ // Split the first basic block
        BasicBlock::iterator iter = oldEntry->end();
        iter--;
        if (oldEntry->size() > 1){
            iter--;
        }
        BasicBlock *splited = oldEntry->splitBasicBlock(iter, Twine("FirstBB"));
        firstbb = splited;
        origBB.insert(origBB.begin(), splited);
    }
    std::vector<Node *> all_node;
    // unsigned int val=0;
    std::vector<unsigned int> rand_list;
    for (std::vector<BasicBlock *>::iterator i = origBB.begin(); i != origBB.end(); i++){
        unsigned int num = getUniqueNumber(&rand_list);
        rand_list.push_back(num);
        Node *tmp = newNode(num);
        all_node.push_back(tmp);
        tmp->data = *i;
    }
    for (std::vector<Node *>::iterator i = all_node.begin(); i != all_node.end(); i++){
        Node *tmp = *i;
        BasicBlock *bb = tmp->data;
        if (bb->getTerminator()->getNumSuccessors() == 2){
            BasicBlock *bb1 = bb->getTerminator()->getSuccessor(0), *bb2 = bb->getTerminator()->getSuccessor(1);
            Node *n1 = findBBNode(bb1, &all_node), *n2 = findBBNode(bb2, &all_node);
            tmp->bb1 = n1;
            tmp->bb2 = n2;
        } else if (bb->getTerminator()->getNumSuccessors() == 1){
            BasicBlock *bb1 = bb->getTerminator()->getSuccessor(0);
            Node *n = findBBNode(bb1, &all_node);
            tmp->bb1 = n;
        }else{
            continue;
        }
        // for(std::vector<Node*>::iterator j=all_node.begin();j!=all_node.end();j++)
    }
    Node *start = findBBNode(firstbb, &all_node);
    Node *fake = newNode(0x7FFFFFFF);
    std::vector<VMInst *> all_inst;
    std::map<Node *, unsigned int> inst_map;
    fake->bb1 = start;
    gen_inst(&all_inst, &inst_map, fake);
    dump_inst(&all_inst);
    std::vector<Constant *> opcodes;
    for (std::vector<VMInst *>::iterator i = all_inst.begin(); i != all_inst.end(); i++){
        VMInst *inst = *i;
        opcodes.push_back(ConstantInt::get(Type::getInt32Ty(f->getContext()), inst->type));
        opcodes.push_back(ConstantInt::get(Type::getInt32Ty(f->getContext()), inst->op1));
        opcodes.push_back(ConstantInt::get(Type::getInt32Ty(f->getContext()), inst->op2));
    }
    ArrayType *AT = ArrayType::get(Type::getInt32Ty(f->getContext()), opcodes.size());
    Constant *opcode_array = ConstantArray::get(AT, ArrayRef<Constant *>(opcodes));
    GlobalVariable *oparr_var = new GlobalVariable(*(f->getParent()), AT, false, GlobalValue::LinkageTypes::PrivateLinkage, opcode_array, "opcodes");
    // 去除第一个基本块末尾的跳转
    oldEntry->getTerminator()->eraseFromParent();
    AllocaInst *vm_pc = new AllocaInst(Type::getInt32Ty(f->getContext()), 0, Twine("VMpc"), oldEntry);
    ConstantInt *init_pc = ConstantInt::get(Type::getInt32Ty(f->getContext()), 0);
    new StoreInst(init_pc, vm_pc, oldEntry);
    AllocaInst *vm_flag = new AllocaInst(Type::getInt32Ty(f->getContext()), 0, Twine("VMJmpFlag"), oldEntry);
    BasicBlock *vm_entry = BasicBlock::Create(f->getContext(), "VMEntry", f, firstbb);

    BranchInst::Create(vm_entry, oldEntry);
    IRBuilder<> IRB(vm_entry);
    Value *zero = ConstantInt::get(Type::getInt32Ty(f->getContext()), 0);

    Value *op1_offset = IRB.CreateAdd(IRB.CreateLoad(vm_pc->getType()->getPointerElementType(), vm_pc), ConstantInt::get(Type::getInt32Ty(f->getContext()), 1));
    Value *op2_offset = IRB.CreateAdd(IRB.CreateLoad(vm_pc->getType()->getPointerElementType(),vm_pc), ConstantInt::get(Type::getInt32Ty(f->getContext()), 2));

    Value *optype = IRB.CreateLoad(IRB.CreateGEP(oparr_var->getType()->getPointerElementType(), oparr_var, {zero, IRB.CreateLoad(vm_pc->getType()->getPointerElementType(), vm_pc)})->getType()->getPointerElementType(),
                                   IRB.CreateGEP(oparr_var->getType()->getPointerElementType(), oparr_var, {zero, IRB.CreateLoad(vm_pc->getType()->getPointerElementType(), vm_pc)}));
    Value *op1 = IRB.CreateLoad(IRB.CreateGEP(oparr_var->getType()->getPointerElementType(), oparr_var, {zero, op1_offset})->getType()->getPointerElementType(),
                                IRB.CreateGEP(oparr_var->getType()->getPointerElementType(), oparr_var, {zero, op1_offset}));

    Value *op2 = IRB.CreateLoad(IRB.CreateGEP(oparr_var->getType()->getPointerElementType(), oparr_var, {zero, op2_offset})->getType()->getPointerElementType(),
                                IRB.CreateGEP(oparr_var->getType()->getPointerElementType(), oparr_var, {zero, op2_offset}));

    IRB.CreateStore(IRB.CreateAdd(IRB.CreateLoad(vm_pc->getType()->getPointerElementType(),vm_pc), ConstantInt::get(Type::getInt32Ty(f->getContext()), 3)), vm_pc);
    BasicBlock *run_block = BasicBlock::Create(f->getContext(), "RunBlock", f, firstbb);
    BasicBlock *jmp_boring = BasicBlock::Create(f->getContext(), "JmpBoring", f, firstbb);
    BasicBlock *jmp_select = BasicBlock::Create(f->getContext(), "JmpSelect", f, firstbb);
    BasicBlock *defaultCase = BasicBlock::Create(f->getContext(), "Default", f, firstbb);
    BranchInst::Create(vm_entry, defaultCase);
    SwitchInst *switch1 = IRB.CreateSwitch(optype, defaultCase, 0);
    switch1->addCase(ConstantInt::get(Type::getInt32Ty(f->getContext()), RUN_BLOCK), run_block);
    switch1->addCase(ConstantInt::get(Type::getInt32Ty(f->getContext()), JMP_BORING), jmp_boring);
    switch1->addCase(ConstantInt::get(Type::getInt32Ty(f->getContext()), JMP_SELECT), jmp_select);

    // create run_block's basicblock
    // the first choice
    IRB.SetInsertPoint(run_block);
    /*
        std::vector<Constant *> bb_addrs;
        for(std::vector<BasicBlock *>::iterator b=origBB.begin();b!=origBB.end();b++){
            BasicBlock *block=*b;
            bb_addrs.push_back(BlockAddress::get(block));
        }
        ArrayType *AT_=ArrayType::get(Type::getInt8PtrTy(f->getContext()),bb_addrs.size());
        Constant *addr_array=ConstantArray::get(AT_,ArrayRef<Constant*>(bb_addrs));
        GlobalVariable *address_arr_var=new GlobalVariable(*(f->getParent()),AT_,false,GlobalValue::LinkageTypes::PrivateLinkage,addr_array,"address_table");
        Value *load=IRB.CreateLoad(IRB.CreateGEP(address_arr_var,{zero,op1}),"address");
        IndirectBrInst *indirBr=IndirectBrInst::Create(load,bb_addrs.size(),run_block);
        for(std::vector<BasicBlock *>::iterator b=origBB.begin();b!=origBB.end();b++)
{
            BasicBlock *block=*b;
            indirBr->addDestination(block);
        }
    */
    // the seconde choice
    SwitchInst *switch2 = IRB.CreateSwitch(op1, defaultCase, 0);
    for (std::vector<BasicBlock *>::iterator b = origBB.begin(); b != origBB.end(); b++){
        BasicBlock *block = *b;
        block->moveBefore(defaultCase);
        Node *t = findBBNode(block, &all_node);
        ConstantInt *numCase = cast<ConstantInt>(ConstantInt::get(switch2->getCondition()->getType(), t->value));
        switch2->addCase(numCase, block);
    }
    for (std::vector<BasicBlock *>::iterator b = origBB.begin(); b != origBB.end(); b++) { // Handle successors
        BasicBlock *block = *b;
        if (block->getTerminator()->getNumSuccessors() == 1){
            errs() << "\033[1;32mThis block has 1 successor\033[0m\n";
            BasicBlock *succ = block->getTerminator()->getSuccessor(0);
            block->getTerminator()->eraseFromParent();
            BranchInst::Create(defaultCase, block);
        } else if (block->getTerminator()->getNumSuccessors() == 2){
            errs() << "\033[1;32mThis block has 2 successors\033[0m\n";
            BranchInst *oldBr = cast<BranchInst>(block->getTerminator());
            SelectInst *select = SelectInst::Create(oldBr->getCondition(), ConstantInt::get(Type::getInt32Ty(f->getContext()), 1), ConstantInt::get(Type::getInt32Ty(f->getContext()), 0), "", block->getTerminator());
            new StoreInst(select, vm_flag, block->getTerminator());
            block->getTerminator()->eraseFromParent();
            BranchInst::Create(defaultCase, block);
        } else {
            continue;
        }
    }
    IRB.SetInsertPoint(jmp_boring);
    IRB.CreateStore(op1, vm_pc);
    IRB.CreateBr(vm_entry);

    IRB.SetInsertPoint(jmp_select);
    BasicBlock *select_true = BasicBlock::Create(f->getContext(), "JmpSelectTrue", f, firstbb);
    BasicBlock *select_false = BasicBlock::Create(f->getContext(), "JmpSelectFalse", f, firstbb);
    IRB.CreateCondBr(IRB.CreateICmpEQ(IRB.CreateLoad(vm_flag->getType()->getPointerElementType(),vm_flag), ConstantInt::get(Type::getInt32Ty(f->getContext()), 1)), select_true, select_false);
    IRB.SetInsertPoint(select_true);
    IRB.CreateStore(op1, vm_pc);
    IRB.CreateBr(vm_entry);
    IRB.SetInsertPoint(select_false);
    IRB.CreateStore(op2, vm_pc);
    IRB.CreateBr(vm_entry);

    std::vector<PHINode *> tmpPhi;
    std::vector<Instruction *> tmpReg;
    BasicBlock *bbEntry = &*f->begin();

    do{
        tmpPhi.clear();
        tmpReg.clear();
        for (Function::iterator i = f->begin(); i != f->end(); i++){
            for (BasicBlock::iterator j = i->begin(); j != i->end(); j++){
                if (isa<PHINode>(j)){
                    PHINode *phi = cast<PHINode>(j);
                    tmpPhi.push_back(phi);
                    continue;
                }
                if (!(isa<AllocaInst>(j) && j->getParent() == bbEntry) && (valueEscapes(&*j) || j->isUsedOutsideOfBlock(&*i))){
                    tmpReg.push_back(&*j);
                    continue;
                }
            }
        }
        for (unsigned int i = 0; i < tmpReg.size(); i++){
            DemoteRegToStack(*tmpReg.at(i), f->begin()->getTerminator());
        }
        for (unsigned int i = 0; i < tmpPhi.size(); i++){
            DemotePHIToStack(tmpPhi.at(i), f->begin()->getTerminator());
        }
    } while (tmpReg.size() != 0 || tmpPhi.size() != 0);
}


VMFlattenPass *llvm::createVMFlatten(bool flag) {
    return new VMFlattenPass(flag);
}