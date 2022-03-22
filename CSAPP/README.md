# CSAPP 深入理解计算机系统

## 0x0 无符号整数 二进制补码
<!--没啥好说的-->
int8
0000 0000 0
1000 0000 -128

uint8
0000 0000 0
1000 0000 128
## 0x1 C语言 位操作 树状数组


``` c++
#include <stdlib.h>
#include <stdio.h>

int main() {
  int a = 1<<31;
  printf("%d\n", a+1);
  printf("0x%x\n", a+1);
  return 0;
}

```

``` c++
#include <stdio.h>
#include <stdlib.h>

unsigned LowBit(unsigned x) {
  unsigned a = x & ((~x) + 1);
  return a;
}

#define MAX (1 << 31) - 1

int main() {
  int a = MAX;
  printf("%d\n", a);
  printf("%d\n", (~a));
  printf("%d\n", (~a) + 1);

  int b =0xa;
  printf("0x%x\n", LowBit(b));
  return 0;
}

```

位操作与线段树

``` c++
include <stdio.h>
#include <stdlib.h>

unsigned LowBit(unsigned x) {
  unsigned a = x & ((~x) + 1);
  return a;
}

int main() {
  printf("0x%x\n", LowBit(12)); // 1100

  unsigned n = 7;
  // segment tree
  printf("S[%u] ==", n);
  printf(" T[%u] +", n);
  n = n - LowBit(n);
  printf(" T[%u] +", n);
  n = n - LowBit(n);
  printf(" T[%u] +", n);
  n = n - LowBit(n);
  printf(" T[%u]\n", n);

  return 0;
}

```

位操作计算某个int的16进制表示是否全是字母

``` c++
>
#include <stdlib.h>

unsigned Letter(unsigned x) {
  unsigned x1 = x & 0x22222222;
  unsigned x2 = x & 0x44444444;
  unsigned x3 = x & 0x88888888;
  return (x3 >> 3) & ((x2 >> 2) | (x1 >> 1));
}

int main() {
  printf("0x%x is Letter 0x%x\n", 0xabcdef, Letter(0xabcdef));
  return 0;
}

```

## 0x2 浮点数表示 类型转换

Floating Point

## 0x3 cpu 寄存器 汇编 指针

## 0x4 汇编 译码 虚拟地址

## 0x5 汇编模拟器 基本框架

## 0x6

## 0x7

## 0x8

## 0x9

## 0xA

## 0xB

## 0xC

## 0xD

## 0xE

## 0xF

## 0x10

## 0x11

## 0x12

## 0x13

## 0x14

## 0x15

## 0x16

## 0x17

## 0x18

## 0x19

## 0x1A

## 0x1B

## 0x1C

## 0x1D

## 0x1E

## 0x1F

## 0x20

## 0x20

## 0x21

## 0x22

## 0x23

## 0x24

## 0x25

## 0x26

## 0x27

## 0x28

## 0x29

## 0x2A

## 0x2B

## 0x2C

## 0x2D

## 0x2E

## 0x2F

## 0x30

## 0x31

## 0x32

## 0x33

## 0x34

## 0x35

## 0x36

## 0x37

## 0x38

## 0x39

## 0x3A

## 0x3B

## 0x3C

## 0x3D

## 0x3E

## 0x3F

