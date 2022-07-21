// 本测试主要适配arm64方向
#include <stdlib.h>
#include <stdio.h>


// 测试其实很好测试 运行demo.sh即可
int add(int a, int b) __attribute((__annotate__(("fla split")))){ // 指定单独某个函数开启xxx混淆 通过注解控制 NEW PM支持目前还不算完善 还在继续探索
    return (a+b);
}

void say_hello() {
    printf("Hello~\n");
}

int main(){
    say_hello();
    int ret = add(10, 20);
    printf("ret is %d\n", ret);
    return 0;
}