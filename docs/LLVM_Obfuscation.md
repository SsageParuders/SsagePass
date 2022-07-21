# LLVM_OBF

- 控制流平坦化 <br>
    增加分发块 <br>
    while + switch
  - 逃逸变量
  - PHI
    > stack中
- 虚假控制流 <br>
    克隆、构造
  - 不可达
  - 不透明谓词
- 随机控制流 <br>
    克隆后,用随机跳转,clone功能相同
- 常量替代 <br>
  - 线性运算
  - 按位运算  

## 指令替代

- ADD a + b <br>
  - addNeg <br>
    > a = b - ( -c )
  - addDoubleNeg <br>
    > a = -( -b + ( -c ))
  - addRand <br>
    > r = rand(); a = b + r; a = a + c; a = a - r
  - addRand2 <br>
    > r = rand(); a = b - r; a = a + b; a = a + r
- SUB
- AND a = b & c <br>
  - andSubstitute <br>
    > a = ( b ^ ~c ) & b <br>
  - andSubstituteRand <br>
    > a = ~ ( ~b | ~c) & ( r | ~r )
- OR a = b | c <br>
  - orSubstitute <br>
    > a = ( b & c ) | ( b ^ c ) <br>
  - orSubstituteRand <br>
    > a = ~ ( ~b & ~c) & ( r | ~r )
- XOR a = b ^ c <br>
  - xorSubstitute <br>
    > a = ( ~a & b ) | ( a & ~b ) <br>
  - xorSubstituteRand <br>
    > a = ( b ^ r ) ^ ( c ^ r ) <br>
    > a = ( ~b & r | b & ~r ) ^ ( ~c & r | c & ~r )

单边看中间,双边相同and不同or,xor全对称
