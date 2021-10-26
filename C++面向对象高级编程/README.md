# C++面向对象高级编程

## 概览

``` c++
#ifndef COMPLEX_H_
#define COMPLEX_H_

class complex{
private:
  double re, im;
public:
  complex (double r = 0, double i = 0) : re (r), im (i) {} // 构造函数
  complex& operator += (const complex&); // 函数声明
  double real () const {return re;}
  double imag () const {return im;}

  friend complex& _doap1 (complex*, const complex&);

};

#endif // COMPLEX_H_

```


> 内联函数 (inline)

函数如果在class body中定义完成，那么就会成为incline函数候选者
``` c++

  double real () const {return re;}
```

> 访问级别

- public 部分调用函数
- private 所有的数据 内部使用的函数
- protect 子类可用

> 构造函数

``` c++
  complex (double r = 0, double i = 0)  // 默认实参
  : re (r), im (i)  // initialization list 这种写法更高效 这里是初始化 后面是赋值
  {
  
  } // 构造函数
```


- 与类名相同
- 没有返回值
- 可以有多个(重载)
- **创建对象时自动调用**

``` c++
int main() {
  complex c1(2, 1);
  complex c2;
  complex *p = new complex(4);
}
```

> 析构函数

不带指针的类大部分不需要析构函数

> 将构造函数放到 `private` 区域中

- 单例模式

``` c++
class A {
public:
  static A& getInstance();
 void  setup();

private:
  A();
  A(const A& rhs);
};

  A& A::getInstance() {
  static A a;
  return a;
}

A::getInstance.setup();

```

> const

不改变类内的数据的时候，应该+ const

``` c++
  double real () const {return re;}
  double imag () const {return im;}
```


如果不加 const 的话

``` c++
  double real () {return re;}
  double imag () {return im;}
```

如果使用者这样使用

``` c++
int main() {
  const complex c(2, 1);
  cout << c.reaol();
  cout << c.imag();
}
```

就会这样报错

``` bash
main.cpp: 在函数‘int main(int, char**)’中:
main.cpp:8:18: 错误：passing ‘const complex’ as ‘this’ argument discards qualifiers [$
rg/onlinedocs/gcc/Warning-Options.html#index-fpermissive-fpermissive]
    8 |   cout << c1.real();
      |           ~~~~~~~^~
In file included from main.cpp:2:
complex.h:12:10: 附注：在调用‘double complex::real()’时
   12 |   double real () {return re;}
      |          ^~~~

```


> 参数传递

- 值传递
尽量不要传递值

- 引用传递
传引用相当于传指针，但是更加优雅

``` c++
complex& complex::operator += (const complex& x);

```


``` c++
operator << (ostream* os, const complex& x) {
return os << '(' << read(x) << ',' << imag(x) << ')';
}
```

> 返回值传递

- 值传递
如果允许，尽量不要传递值

- 引用传递
传引用相当于传指针，但是更加优雅

`+=` 可以返回引用
``` c++
complex& complex::operator += (const complex& x) {
  this->re += x.re;
  this->im += x.im;
  return *this;
}
```

`+` 就不可以返回引用

``` c++
complex& complex::operator + (const complex& x) {
  complex c(this->re + x.re, this->im + x.im);
  return c; // 这样就会返回一个局部变量的引用 是无效的
}

complex.cpp:11:10: 警告：reference to local variable ‘c’ returned [$
Warning-Options.html#index-Wreturn-local-addr-Wreturn-local-addr]
   11 |   return c; // 这样就会返回一个局部变量的引用 是无效的
      |          ^
complex.cpp:10:11: 附注：在此声明
   10 |   complex c(this->re + x.re, this->im + x.im);
      |           ^

```

``` c++
complex complex::operator + (const complex& x) {
  complex c(this->re + x.re, this->im + x.im)
  return c;
}
```

return by reference的好处还有传递着无需知道接收者是以reference形式接收

> friend(友元)

``` c++
  friend complex& _doap1 (complex*, const complex&);
```

``` c++
inline complex& _doapl (complex* ths, const complex& r){
  ths->re += r.re;
  ths->im += r.im;
  return *ths;
}

```

同一个class的object之间互为友元

## 操作符重载与临时对象

- 在c++中，**操作符其实是一种函数**
- this 是一个`指针`

``` c++
#include "complex.h"

inline complex& complex::operator += (const complex& r) {
  return _doapl(this, r);
}

inline complex& _doapl (complex* ths, const complex& r){
  ths->re += r.re;
  ths->im += r.im;
  return *ths;
}

```

> 临时对象

``` c++

inline double imag(const complex& x){
  return x.imag();
}

inline double real(const complex& x) {
  return x.real();
}

complex operator + (const complex& x, const complex& y) {
  return complex(real(x) + real(y), imag(x) + imag(y));
}

complex operator + (const complex& x, const double y) {
  return complex(real(x) + y, imag(x) + y);
}

complex operator + (const double x, const complex& y) {
  return complex(x + real(y), x + imag(y));
}

```


### 复习





## 三大函数






## 堆 栈 与内存管理






