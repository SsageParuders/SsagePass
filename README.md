>
> Port to LLVM 14.01<br>
> Docker for llvm environment: ssageparuders/ubuntu_llvm_14_01_pass<br>
> Made by SsageParuders<br>
>

# HowToUse



# [Source Code](./Obfuscation/)

> 

# [Demo For Test](./Demo/)

> 

```shell
chmod +x demo.sh && ./demo.sh
```

# [Docs](./docs/)

> 

# TODO LIST

- [x] 实现对单个Function启用PASS

- [x] 完善LLVM API文档

- [x] 测试Hikari的14适配

- [x] 解决LowerSwitchPass在LLVM-9以上的适配问题

- [x] 更换PASS管理器为`NEW PM`

- [ ] 解决`SplitNum`混淆程度在`NEW PM`上的传递问题

- [ ] 解决`NEW PM`中 clang如何触发指定PASS的功能

- [ ] 适配上Android编译链

- [ ] 完善README和Docs文档

- [ ] 完善英文文档