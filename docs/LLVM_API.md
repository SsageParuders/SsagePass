# LLVM_API

> [官方文档](https://llvm.org/docs/ProgrammersManual.html#important-and-useful-llvm-apis)

## 常用LLVM API解释

---

### 三个核心类

#### Function 函数 类

> 与Function有关的操作主要是获取函数的属性<br>
> 对函数中的基本块进行遍历

```cpp
bool runOnFunction(Function &F){
    F.getName(); // 获取函数的symbols
    F.getEntryBlock(); // 获取函数的入口
    for(BasicBlock &BB : F){ // 对函数中的基本块进行遍历
        // TODO
    }
}
```

#### BasicBlock 基本块 类

> 与BasicBlock有关的操作主要是获取基本块的属性<br>
> 对基本块中的指令进行遍历<br>
> 对基本块作克隆 分裂 移动处理

```cpp
bool runOnFunction(Function &F){
    for(BasicBlock &BB : F){ // 对函数中的基本块进行遍历
        BB.getName(); // 获取基本块的名称
        BB.getTerminator(); // 获取基本块的终结指令
        for(Instruction &I : BB){ // 对基本块中的指令进行遍历
            // TODO
        }
    }
}
```

#### Instruction 指令 类

> 指令有很多中 所以`Instruction`有很多的子类<br>
> AllocaInst BranchInst ...<br>
> 有关操作主要是 指令的创建 删除 修改<br>
> 以及指令中操作数的遍历

```cpp
bool runOnFunction(Function &F){
    for(BasicBlock &BB : F){ // 对函数中的基本块进行遍历
        for(Instruction &I : BB){ // 对基本块中的指令进行遍历
            for(int i =0; i < I.getNumOperands(); i++){ // 对操作数的遍历
                // TODO
            }
        }
    }
}
```

---

### 一个基本类: Value

> 所有可以被当作`指令操作数`的类型都是Value的子类<br>
> Value有以下五种类型的子类:<br>
>
> - 常量 Constant
> - 参数 Argument
> - 指令(的运算结果) Instruction
> - 函数(的指针) Function
> - 基本块 BasicBlock

---

### 输出流

```cpp
// LLVM 中 使用 outs() errs() dbgs() 打印输出流
bool runOnFunction(Function &F){
    // 一般信息打印
    outs() << "Hello\n" << "\n";
    // 错误信息打印
    errs() << "Hello\n" << "\n";
    // 调试信息打印
    dbgs() << "Hello\n" << "\n";
}
```

---

## 个人笔记

---
