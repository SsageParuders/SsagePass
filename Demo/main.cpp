// 本测试主要适配arm64方向
#include <stdlib.h>
#include <stdio.h>

int add(int a, int b) __attribute((__annotate__(("funwra bcf fla strenc indibr")))){
    return (a+b);
}
// strenc  indibr strenc
void say_hello() __attribute((__annotate__(("funwra bcf fla strenc indibr")))){
    printf("Hello~\n");
}

int main(){
    say_hello();
    int ret = add(10, 20);
    printf("ret is %d\n", ret);
    return 0;
}