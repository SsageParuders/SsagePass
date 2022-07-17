cd ./Demo # 进入Demo文件夹

ls | grep -v main.cpp | xargs rm # 清除上一次产物

echo "==================== Start Test Fla ===================="
clang -S -emit-llvm main.cpp -o main.ll # 编译成可阅读的IR层
opt -load ../build/SsageObfuscator.so -enable-new-pm=0 -split -split_num=7 -S main.ll -o main_split.ll # 使用opt优化中间pass
## 这里LLVM组织不再维护传统的`Legacy Pass Manager`而是采用了新的PASS管理器
## 并且默认关闭传统的PASS管理器 启用新的PASS管理器
## 因此, 如果PASS的写法如果还是沿用传统写法
## 则需要对opt传入`-enable-new-pm=0`,关闭新的PASS管理器,启用传统的PASS管理器 // clang 传入 -flegacy-pass-manager
## 或者可以使用新的PASS管理器的写法
## 参考1:https://groups.google.com/g/llvm-dev/c/kQYV9dCAfSg
## 参考2:https://releases.llvm.org/14.0.0/docs/ReleaseNotes.html#changes-to-the-llvm-ir
## 参考3:https://llvm.org/docs/WritingAnLLVMPass.html#introduction-what-is-a-pass
clang main_split.ll -o main_split # 正式编译为可执行程序
./main_split # 可执行程序执行
echo "==================== Finish Test Fla ===================="

echo "==================== Start Test Fla ===================="
clang -S -emit-llvm main.cpp -o main.ll # 编译成可阅读的IR层
# opt -lowerswitch -S main.ll -o main_lowerswitch.ll
# opt -load ../Build/SsageObfuscator.so -enable-new-pm=0 -fla -split_num=7 -S main_lowerswitch.ll -o main_fla.ll
opt -load ../Build/SsageObfuscator.so -enable-new-pm=0 -fla -split_num=7 -S main.ll -o main_fla.ll
clang main_fla.ll -o main_fla
./main_fla
echo "==================== Finish Test Fla ===================="