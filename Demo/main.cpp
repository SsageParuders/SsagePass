// 本测试主要适配arm64方向
#include <stdlib.h>
#include <stdio.h>

static int aaa = 88;

int add(int a, int b) __attribute((__annotate__(("split vmf")))){
    return (a+b);
}
// strenc  indibr strenc
void say_hello()
    // indibr split vmf
    __attribute((__annotate__(("icall"))))
{
    printf("Hello~\n");
}

int main(){
    say_hello();
    int ret = add(10, 20);
    printf("ret is %d\n", ret);
    printf("aaa is %d\n",aaa);
    return 0;
}