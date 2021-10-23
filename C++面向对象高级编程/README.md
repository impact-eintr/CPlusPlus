# C++面向对象高级编程

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
``` c++

  double real () const {return re;}
```

