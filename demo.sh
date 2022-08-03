cd ./Demo # 进入Demo文件夹

ls | grep -v main.cpp | grep -v help.txt | grep -v symbols_obf.yaml | xargs rm # 清除上一次产物

echo "==================== Start Test With clang ===================="
clang++ \
    -fpass-plugin=../build/libSsageObfuscator.so \
    -mllvm --rewrite-map-file=symbols_obf.yaml \
    -Xclang -load -Xclang ../build/libSsageObfuscator.so \
    -mllvm -split_num=7 \
    -mllvm -bcf_prob=100 -mllvm -bcf_loop=3 -mllvm -bcf_cond_compl=5 \
    -mllvm -fw_prob=100 -mllvm -fw_times=4 \
    main.cpp -o main
./main
# sleep 1s
readelf -s main
echo "==================== Finish Test With clang ===================="

## 仅学习阶段实践了 Legacy Pass Manager 后续已迁移到 NEW Pass Manager
# echo "==================== Start Test Fla ===================="
# clang -S -emit-llvm main.cpp -o main.ll # 编译成可阅读的IR层
# opt -load ../build/SsageObfuscator.so -enable-new-pm=0 -split -split_num=7 -S main.ll -o main_split.ll # 使用opt优化中间pass
# ## 这里LLVM组织不再维护传统的`Legacy Pass Manager`而是采用了新的PASS管理器
# ## 并且默认关闭传统的PASS管理器 启用新的PASS管理器
# ## 因此, 如果PASS的写法如果还是沿用传统写法
# ## 则需要对opt传入`-enable-new-pm=0`,关闭新的PASS管理器,启用传统的PASS管理器 // clang 传入 -flegacy-pass-manager
# ## 或者可以使用新的PASS管理器的写法
# ## 参考1:https://groups.google.com/g/llvm-dev/c/kQYV9dCAfSg
# ## 参考2:https://releases.llvm.org/14.0.0/docs/ReleaseNotes.html#changes-to-the-llvm-ir
# ## 参考3:https://llvm.org/docs/WritingAnLLVMPass.html#introduction-what-is-a-pass
# clang main_split.ll -o main_split # 正式编译为可执行程序
# ./main_split # 可执行程序执行
# echo "==================== Finish Test Fla ===================="

# echo "==================== Start Test Fla With opt ===================="
# clang -S -emit-llvm main.cpp -o main.ll # 编译成可阅读的IR层
# # opt -load ../build/SsageObfuscator.so -enable-new-pm=0 -fla -split_num=7 -S main.ll -o main_fla.ll # 传统写法
# opt --load-pass-plugin=../build/SsageObfuscator.so -O1 -S main.ll -o main_fla.ll
# # opt --load-pass-plugin=../build/SsageObfuscator.so -O1 -S main.ll -o main_fla.ll # 开启全部
# # opt --load-pass-plugin=../build/SsageObfuscator.so -passes=split,fla -load ../Build/SsageObfuscator.so -split_num=7 -S main.ll -o main_fla.ll
# # 一种传递混淆程度的临时方案
# clang main_fla.ll -o main_fla
# ./main_fla
# echo "==================== Finish Test Fla With opt ===================="

# echo "==================== Start Test Fla With clang ===================="
# clang++ -fpass-plugin=../build/SsageObfuscator.so main.cpp -o main # 可能需要 -O1 
# # clang++ -fpass-plugin=../build/SsageObfuscator.so -Xclang -load -Xclang ../build/SsageObfuscator.so -mllvm -split_num=7 main.cpp -o main
# ## 参考:https://github.com/banach-space/llvm-tutor#overview-of-the-passes 的下半段
# ./main
# echo "==================== Finish Test Fla With clang ===================="


# echo "==================== Start Test strenc With opt ===================="
# clang -S -emit-llvm main.cpp -o main.ll # 编译成可阅读的IR层
# # opt -load ../build/SsageObfuscator.so -enable-new-pm=0 -fla -split_num=7 -S main.ll -o main_fla.ll # 传统写法
# opt --load-pass-plugin=../build/libSsageObfuscator.so -O1 -S main.ll -o main_strenc.ll
# # opt --load-pass-plugin=../build/SsageObfuscator.so -O1 -S main.ll -o main_fla.ll # 开启全部
# # opt --load-pass-plugin=../build/SsageObfuscator.so -passes=split,fla -load ../Build/SsageObfuscator.so -split_num=7 -S main.ll -o main_fla.ll
# # 一种传递混淆程度的临时方案
# clang main_strenc.ll -o main_strenc
# ./main_strenc
# echo "==================== Finish Test strenc With opt ===================="

# echo "==================== Start Test Fla With clang ===================="
# clang++ -fpass-plugin=../build/SsageObfuscator.so main.cpp -o main # 可能需要 -O1 
# # clang++ -fpass-plugin=../build/SsageObfuscator.so -Xclang -load -Xclang ../build/SsageObfuscator.so -mllvm -split_num=7 main.cpp -o main
# ## 参考:https://github.com/banach-space/llvm-tutor#overview-of-the-passes 的下半段
# ./main
# echo "==================== Finish Test Fla With clang ===================="