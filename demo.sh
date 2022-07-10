cd ./Demo # 进入Demo文件夹
rm main.ll
rm main_hlw.ll
rm main_hlw
clang -S -emit-llvm main.cpp -o main.ll # 编译成可阅读的IR层
opt -load ../build/LLVMObfuscator.so -enable-new-pm=0 -hlw -S main.ll -o main_hlw.ll # 使用opt优化中间pass
## 这里LLVM组织不再维护传统的`Legacy Pass Manager`而是采用了新的PASS管理器
## 并且默认关闭传统的PASS管理器 启用新的PASS管理器
## 因此, 如果PASS的写法如果还是沿用传统写法
## 则需要对opt传入`-enable-new-pm=0`,关闭新的PASS管理器,启用传统的PASS管理器 // clang 传入 -flegacy-pass-manager
## 或者可以使用新的PASS管理器的写法
## 参考1:https://groups.google.com/g/llvm-dev/c/kQYV9dCAfSg
## 参考2:https://releases.llvm.org/14.0.0/docs/ReleaseNotes.html#changes-to-the-llvm-ir
clang main_hlw.ll -o main_hlw # 正式编译为可执行程序
./main_hlw # 可执行程序执行