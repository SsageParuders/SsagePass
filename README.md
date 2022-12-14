>
> Port to LLVM 14.06<br>
> Docker for llvm environment: ssageparuders/android_llvm_14.06:18.04<br>
> Made by SsageParuders<br>
>

# HowToUse

> SsagePass为out-of-tree编译的LLVM动态库插件<br>

- 各参数介绍:
    ```bash
    # __attribute((__annotate__(("xx")))) // 填写注解 以控制单个函数的Pass
    ##-----======= 详解 =======-----##
    # bcf --- 虚假控制流
    ## bcf_prob -- 每个基本块被虚假控制流混淆的概率 -- 0 < bcf_prob < 100
    ## bcf_loop -- 每个函数被虚假控制流重复多少次 -- 无限制 建议 2～5
    ## bcf_cond_compl -- 用于生成分支条件的表达式的复杂性 -- 无限制 建议 3～10
    # ofla --- 控制流平坦化
    # enfla --- 控制流平坦化增强版
    # mba --- 线性混合布尔算术混淆
    # funwra --- 函数嵌套包装
    ## fw_prob -- 每个函数被包装的可能性 -- 0 < fw_prob < 100
    ## fw_times -- 每个函数被包装嵌套多少次 -- 无限制 建议 2～5
    # split -- 基本块分割
    ## split_num -- 原先一个基本块被分割为多少基本块 -- 无限制 建议 2～5
    # indibr --- 间接跳转
    # vmf --- 虚拟机控制流平坦化
    # strenc --- 字符串加密
    ```
    > 以上介绍`#`标记的为注解填写 用于控制混淆开关<br>
    > `##`标记的为cl::opt参数 用于传递混淆粒度

- 替代symbols
    > 该功能是llvm自带的 只不过我专门注册一下罢
    ```bash
    # symbols_obf.yaml
    # function: { source: _Z3addii, target: dfffff} # 函数替换
    # global variable: { source: _ZL3aaa, transform: ccccc} # 变量替换
    clang++ -fpass-plugin=../build/libSsageObfuscator.so -mllvm --rewrite-map-file=symbols_obf.yaml main.cpp -o main
    # 通过-mllvm --rewrite-map-file=symbols_obf.yaml传递替换信息进入编译
    ```

- clang中触发指定Pass的临时方案:<br>

    ```bash
    # opt样例 -- 默认开启全部Pass 但是是否真的启用 依然需要读取并且判断函数注解
    opt --load-pass-plugin=../build/SsageObfuscator.so -O1 -S main.ll -o main_fla.ll
    # opt样例 -- 指定开启全局某Pass 但是是否真的启用某Pass 依然需要读取并且判断函数注解
    opt --load-pass-plugin=../build/SsageObfuscator.so -passes=split,fla -S main.ll -o main_fla.ll
    # clang样例
    clang++ -fpass-plugin=../build/SsageObfuscator.so main.cpp -o main
    ```
    > 在clang的NEW PM中 我始终无法成功通过传入特定参数触发指定Pass<br>
    > 因此 我在代码的PMRegistration.cpp里 默认注册全部的Pass 并且默认为不开启<br>
    > 只有读取函数注解 成功匹配到相应Pass的字符串 才会对相应函数开启特定Pass<br>
    > 这里也建议其他使用者 非必要 不开启全局混淆 这会导致不必要的性能损耗<br>
    > 针对关键函数启用指定混淆 这种方案在我眼中最佳

- 传递`SplitNum`这种混淆粒度的临时解决方案:<br>
    *把动态库用两种方案都加载一遍，但是Pass用NEW PM控制*
    ```bash
    # opt样例
    opt --load-pass-plugin=../build/SsageObfuscator.so -passes=split,fla -load ../Build/SsageObfuscator.so -split_num=7 -S main.ll -o main_fla.ll
    # clang样例
    clang++ -fpass-plugin=../build/SsageObfuscator.so -Xclang -load -Xclang ../build/SsageObfuscator.so -mllvm -split_num=7 main.cpp -o main
    ```
    > 在clang的NEW PM中 [貌似暂时不支持传递cl::opt内容](https://github.com/banach-space/llvm-tutor/blob/c69893afc6248a5b3c72d38a8ec5600a50f92bb4/lib/MBAAdd.cpp#L12-L19)<br>
    > 不知道以后会不会优化 或者是有什么其他较优方案<br>
    > 然后又因为我们什么地方启用什么Pass完全是由函数的注解控制的<br>
    > 所以我们只需要用NEW PM的方案加载进Pass插件即可使其生效<br>
    > 然后再用Legacy Pass Manager的方案载入插件<br>
    > 这样传入cl::opt内容 如此可以指定混淆程度

- 什么是函数注解:<br>
    ```cpp
    // 样例 如果熟悉ollvm应该一下子就知道是什么意思了
    void say_hello() __attribute((__annotate__(("fla split strenc")))){
        printf("Hello~\n");
    }
    ```

> 这两种临时方案都是受限于个人水平有限的无奈之举<br>
> 如果有人知道如何更好的解决 欢迎提交PR

# [Source Code](./Obfuscation/)

> LLVM Pass的源代码<br>
> 本Pass采用out-of-tree方式编译为动态库<br>
> 以方便作为插件便捷载入

# [Demo For Test](./Demo/)

> 用于测试的代码 后续会添加Android的测试样本

```shell
chmod +x demo.sh && ./demo.sh
```

# [Docs](./docs/)

> 学习LLVM过程中的一些笔记和个人积累

---

# TODO LIST

- [x] 实现对单个Function启用PASS

- [x] 初步完善LLVM API文档

- [x] 测试Hikari的14适配

- [x] 解决LowerSwitchPass在LLVM-9以上的适配问题

- [x] 更换PASS管理器为`NEW PM`

- [x] 适配来自Hikari的字符串加密

- [x] 解决`SplitNum`混淆程度在`NEW PM`上的传递问题

- [x] 解决`NEW PM`中 clang如何触发指定PASS的功能

- [x] 初步完善README和Docs文档

- [x] 初步适配上Android编译链[ndk_r25]

- [x] 适配来自Hikari的间接跳转

- [x] 适配来自Hikari的函数包装

- [x] 适配Hikari优化过的虚假控制流

- [x] 优化函数包装为随机字符串

- [ ] 完善控制流平坦化

- [ ] 完善英文文档

---

# Credits

[OLLVM](https://github.com/heroims/obfuscator) By heroims

[llvm-pass-tutorial](https://github.com/LeadroyaL/llvm-pass-tutorial) By LeadroyaL

[llvm-tutor](https://github.com/banach-space/llvm-tutor) By banach-space

[Pluto-Obfuscator](https://github.com/bluesadi/Pluto-Obfuscator) By bluesadi

[goron](https://github.com/amimo/goron) By amimo

[Hikari](https://github.com/HikariObfuscator/Hikari) By HikariObfuscator

[LLVMMyPass](https://github.com/za233/LLVMMyPass) By za233

---