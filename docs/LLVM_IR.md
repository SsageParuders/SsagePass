# LLVM_IR

> [官方文档](https://llvm.org/docs/LangRef.html#high-level-structure)

## IR结构

- 模块 Module <br>
    1. 头部信息 <br>
        目标平台等
    2. 全局符号 <br>
        函数定义、函数声明、全局变量
- 函数 Function <br>
    1. 参数
    2. 入口块
    3. 其他基本块
- 基本块 BasicBlock <br>
    1. 标签
    2. PHI指令
    3. 其他指令
    4. 终结指令 <br>
        `br、switch、retn`

### 混淆层次 <br>

- 以函数为单位 <br>
    控制流平坦化
- 以基本块为单位 <br>
    虚假控制流
- 以指令为基本单位 <br>
    指令替代

## 常用LLVM IR指令解释

---

### 终结指令 LLVM IR 指令

- ret 函数的返回指令<br>
    对应C/C++的`return`<br>
    1. 需要指定返回值的类型和值的ret指令(可以返回一整个结构体) <br>
        `ret \<type> \<value>` <br>
        > ret { i64, i32} { i64, 8, i32 8}
    2. 无返回值的ret指令 <br>
        `ret void`

- br 分支指令<br>
    对应C/C++的`if`<br>
    > 分为非条件分支和条件分支指令
    1. 无条件分支指令类似于x86的`jmp`指令
    2. 条件分支类似于x86汇编的`jnz`或`je`指令 <br>
        `br i1 \<cond>, label \<true>, label \<false>`

- 补充:比较指令
    1. icmp:整数或指针的比较指令<br>
        `\<result> = icmp \<cond> \<ty> \<op1> \<op2> <br>` <br>
        cond 条件可为: eq 相等 ｜ ne 不相等 | ugt 无符号大于 | ... <br>
        > result = icmp eq i32 1,1 ; yields: result = false

    2. fcmp:浮点数的比较指令<br>
        cond 条件可为: oeq 相等并且两个操作数不可为NAN ｜ ueq 相等 | ...<br>
        > o(ordered): 两个操作数都不可为NAN 否则返回false

- switch 分支指令<br>
    相当于C/C++的`switch`指令<br>
    可以看作是br指令的升级版 支持的分支更多<br>
    `switch \<intty> \<value>, lable \<defaultdest> [val:dest ...]`

---

### 二元运算 LLVM IR 指令

- add 整数的加法指令<br>
    对应C/C++的`+` <br>
    `\<result> = add nuw nsw \<ty> \<op1>, \<op2>` <br>
    > nuw : No Unsigned Wrap <br>
    > nsw : No Signed Wrap

- sub 整数的减法指令<br>
    对应C/C++的`-`

- mul 整数的乘法指令<br>
    对应C/C++的`*`

- udiv 无符号整数除法指令<br>
    对应C/C++的`/`<br>
    `\<result> = udiv exact \<ty> \<op1>, \<op2>` <br>
    > 如果有exact(精确)关键字修饰 则如果op1不是op2的倍数 则出错 <br>

- sdiv 有符号的整数除法指令<br>
    对应C/C++的`/`<br>
    > 如果有exact(精确)关键字修饰 则如果op1不是op2的倍数 则出错

- urem 无符号整数的取余数指令<br>
    对应C/C++`%`

- srem 有符号整数的取余指令<br>
    对应C/C++`%`

---

### 按位二元运算 LLVM IR 指令

- shl 整数左移指令<br>
    对应C/C++的`<<`

- lshr 整数逻辑右移指令<br>
    对应C/C++的`>>`<br>
    右移指定位数后 左侧补0
    > 不管里面的操作数是有符号的还是无符号的 <br>
    > 统一当成无符号的数进行处理 -- 逻辑右移

- ashr 整数的算术右移指令<br>
    对应C/C++的`>>`<br>
    右移指定位数后 左侧补符号位<br>
    > 负数补1 正数补0

- and 整数的按位运算指令<br>
    对应C/C++的`&`

- or 整数的按位或运算指令<br>
    对应C/C+的`|`

- xor 整数的按位异或运算指令<br>
    对应C/C++的`^`

---

### 内存访问和寻址操作

> 静态单赋值(SSA): 程序中一个变量只能有一条赋值语句

- alloca 内存分配指令<br>
    在栈中分配一段空间并获得指向改空间的指针<br>
    对应C/C++的`malloc` <br>
    `\<result> = alloca \<type> [, \<ty> \<NumElements>] [, align \<alignment>]` <br>
    > 分配sizeof(type)*NumElements字节, 并与alignment对齐

- store 内存储存指令<br>
    向指针指向的内存中存储数据<br>
    类似于C/C++中的指针解引用后的赋值操作 <br>
    `store \<ty> \<value>, \<ty>\* \<pointer>` <br>
    > 写入value到ptr

- load 内存读取指令<br>
    从指针指向的内存中读取数据<br>
    类似于C/C++中的指针解引用操作 <br>
    `load \<ty> \<value>, \<ty>\* \<pointer>`

---

### 类型转化操作

- trunc ... to 截断指令<br>
    将一种变量截断为另一种变量<br>
    对应C/C++中大类型向小类型的强制转化<br>
    > long --> int

- zext ... to 零拓展指令<br>
    将一种类型的变量拓展为另一种变量 高位补0<br>
    对应C/C++中小类型向大类型的强制转化<br>
    > int --> long

- sext ... to 符号位拓展<br>
    通过复制符号位(最高位) 将一种类型的变量拓展为另一种类型的变量<br>
    > When sign extending from i1, the extension always results in -1 or 0.

---

### 其他操作

- phi SSA衍生指令<br>
    为了解决SSA一个变量只能被赋值一次而引起问题的衍生指令<br>
    phi指令的计算结果由phi指令所在基本块的前驱块确定<br>
    `\<result> = phi \<ty> [\<val0>, \<lable0>], ...`

- select <br>
    类似于C/C++的三元运算符`...?...:...`

- call <br>
    用来调用某函数<br>
    与C/C++中的函数调用类似<br>

---

## 个人笔记

---
