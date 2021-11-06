# const那些事

## 1.const含义
常类型是指使用类型修饰符const说明的类型，常类型的变量或对象的值是不能被更新的。

## 2.const作用
- 可以定义常量
``` c++
const int a = 100;
```

- 类型检查
const常量与#define宏定义常量的区别：~~const常量具有类型，编译器可以进行安全检查；#define宏定义没有数据类型，只是简单的字符串替换，不能进行安全检查~~。

- 防止修改，起保护作用，增加程序健壮性
``` c++
void f(const int i) {
    i++; // error
}
```

- 可以节省空间，避免不必要的内存分配
const定义常量从汇编的角度来看，只是给出了对应的内存地址，而不是像#define一样给出的是立即数，所以，const定义的常量在程序运行过程中只有一份拷贝，而#define定义的常量在内存中有若干个拷贝。

## 3.const对象默认为文件局部变量
**注意：非const变量默认为extern。要使const变量能够在其他文件中访问，必须在文件中显式地指定它为extern。**

``` c++
// file1.cpp
int ext
// file2.cpp
#include<iostream>
/**
 * by eintr
 * compile: g++ -o file file2.cpp file1.cpp
 * execute: ./file
 */
extern int ext;

int main(){
    std::cout<<(ext+10)<<std::endl;
}

```

**小结：可以发现未被const修饰的变量不需要extern显式声明！而const常量需要显式声明extern，并且需要做初始化！因为常量在定义后就不能被修改，所以定义时必须初始化。**

## 4.定义常量
``` c++
const int b = 10;
b = 0; // error b为常量，不可更改
const string s "helloworld";
const int i, j = 0; // i为常量，必须进行初始化！(因为常量在定义后就不能被修改，所以定义时必须初始化。)
```


## 5.指针与const

``` c++
const char * a; //指向const对象的指针或者说指向常量的指针。
char const * a; //同上
char * const a; //指向类型对象的const指针。或者说常指针、const指针。
const char * const a; //指向const对象的const指针。
```

``` c++
#include <iostream>

using namespace std;

int main() {
  int i = 12;
  int j = 10;

  const int *ip = &i;
  ip = &j;

  int const *ip_ = &i;
  ip_ = &j;

  int * const iq = &i;
  *iq = j;

  const int * const ipq = &i;
}

```

小结：如果const位于*的左侧，则const就是用来修饰指针所指向的变量，即指针指向为常量；如果const位于*的右侧，const就是修饰指针本身，即指针本身是常量。

具体使用如下：

- （1）指向常量的指针
``` c++
const int *ptr;
*ptr = 10; //error

```

ptr是一个指向int类型const对象的指针，const定义的是int类型，也就是ptr所指向的对象类型，而不是ptr本身，所以ptr可以不用赋初始值。但是不能通过ptr去修改所指对象的值。

除此之外，也不能使用void*指针保存const对象的地址，必须使用const void*类型的指针保存const对象的地址。

``` c++
const int p = 10;
const void * vp = &p;
void *vp = &p; //error
```

另外一个重点是：允许把非const对象的地址赋给指向const对象的指针。

将非const对象的地址赋给const对象的指针:

``` c++
const int *ptr;
int val = 3;
ptr = &val; //ok
```

我们不能通过ptr指针来修改val的值，即使它指向的是非const对象!

我们不能使用指向const对象的指针修改基础对象，然而如果该指针指向了非const对象，可用其他方式修改其所指的对象。可以修改const指针所指向的值的，但是不能通过const对象指针来进行而已！如下修改：

``` c++
int *ptr1 = &val;
*ptr1=4;
cout<<*ptr<<endl;
```

小结：对于指向常量的指针，不能通过指针来修改对象的值。
也不能使用void`*`指针保存const对象的地址，必须使用const void`*`类型的指针保存const对象的地址。
允许把非const对象的地址赋值给const对象的指针，如果要修改指针所指向的对象值，必须通过其他方式修改，不能直接通过当前指针直接修改。

- （2）常指针

const指针必须进行初始化，且const指针的值不能修改。

``` c++
#include<iostream>
using namespace std;
int main(){

    int num=0;
    int * const ptr=&num; //const指针必须初始化！且const指针的值不能修改
    int * t = &num;
    *t = 1;
    cout<<*ptr<<endl;
}
```

上述修改ptr指针所指向的值，可以通过非const指针来修改。

最后，当把一个const常量的地址赋值给ptr时候，由于ptr指向的是一个变量，而不是const常量，所以会报错，出现：const int* -> int *错误！

``` c++
#include<iostream>
using namespace std;
int main(){
    const int num=0;
    int * const ptr=&num; //error! const int* -> int*
    cout<<*ptr<<endl;
}
```

上述若改为 const int *ptr或者改为const int *const ptr，都可以正常！

- （3）指向常量的常指针

理解完前两种情况，下面这个情况就比较好理解了：

``` c++
const int p = 3;
const int * const ptr = &p; 
```

ptr是一个const指针，然后指向了一个int 类型的const对象。

## 6.函数中使用const

> cost修饰函数返回值

（1）const int

``` c++
const int func1();
```

这个本身无意义，因为参数返回本身就是赋值给其他的变量！

（2）const int*

``` c++
const int* func2();
```

指针指向的内容不变。

（3）int *const

``` c++
int *const func2();
```

指针本身不可变。

> const修饰函数参数

- （1）传递过来的参数及指针本身在函数内不可变，无意义！

``` c++
void func(const int var); // 传递过来的参数不可变
void func(int *const var); // 指针本身不可变
```

表明参数在函数体内不能被修改，但此处没有任何意义，var本身就是形参，在函数内不会改变。包括传入的形参是指针也是一样。

输入参数采用“值传递”，由于函数将自动产生临时变量用于复制该参数，该输入参数本来就无需保护，所以不要加const 修饰。

- （2）参数指针所指内容为常量不可变

``` c++
void StringCopy(char *dst, const char *src);
```

其中src 是输入参数，dst 是输出参数。给src加上const修饰后，如果函数体内的语句试图改动src的内容，编译器将指出错误。这就是加了const的作用之一。

- （3）参数为引用，为了增加效率同时防止修改。

``` c++
void func(const A &a)
```

对于非内部数据类型的参数而言，象void func(A a) 这样声明的函数注定效率比较低。因为函数体内将产生A 类型的临时对象用于复制参数a，而临时对象的构造、复制、析构过程都将消耗时间。

为了提高效率，可以将函数声明改为void func(A &a)，因为“引用传递”仅借用一下参数的别名而已，不需要产生临时对象。但是函数void func(A &a) 存
在一个缺点：

“引用传递”有可能改变参数a，这是我们不期望的。解决这个问题很容易，加const修饰即可，因此函数最终成为

``` c++
void func(const A &a)。
```

以此类推，是否应将void func(int x) 改写为void func(const int &x)，以便提高效率？完全没有必要，因为内部数据类型的参数不存在构造、析构的过程，而复制也非常快，“值传递”和“引用传递”的效率几乎相当。

**小结：对于非内部数据类型的输入参数，应该将“值传递”的方式改为“const 引用传递”，目的是提高效率。例如将void func(A a) 改为void func(const A &a)。
对于内部数据类型的输入参数，不要将“值传递”的方式改为“const 引用传递”。否则既达不到提高效率的目的，又降低了函数的可理解性。例如void func(int x) 不应该改为void func(const int &x)。**

## 7.类中使用const

在一个类中，任何不会修改数据成员的函数都应该声明为const类型。如果在编写const成员函数时，不慎修改数据成员，或者调用了其它非const成员函数，编译器将指出错误，这无疑会提高程序的健壮性。使用const关字进行说明的成员函数，称为常成员函数。只有常成员函数才有资格操作常量或常对象，没有使用const关键字声明的成员函数不能用来操作常对象。

对于类中的const成员变量必须通过初始化列表进行初始化，如下所示：

``` c++
class Apple
{
private:
    int people[100];
public:
    Apple(int i); 
    const int apple_number;
};

Apple::Apple(int i):apple_number(i)
{

}
```

**const对象只能访问const成员函数,而非const对象可以访问任意的成员函数,包括const成员函数.**

``` c++
//apple.cpp
class Apple
{
private:
    int people[100];
public:
    Apple(int i); 
    const int apple_number;
    void take(int num) const;
    int add(int num);
    int add(int num) const;
    int getCount() const;

};
//main.cpp
#include<iostream>
#include"apple.cpp"
using namespace std;

Apple::Apple(int i):apple_number(i)
{

}
int Apple::add(int num){
    take(num);
}
int Apple::add(int num) const{
    take(num);
}
void Apple::take(int num) const
{
    cout<<"take func "<<num<<endl;
}
int Apple::getCount() const
{
    take(1);
//    add(); //error
    return apple_number;
}
int main(){
    Apple a(2);
    cout<<a.getCount()<<endl;
    a.add(10);
    const Apple b(3);
    b.add(100);
    return 0;
}
//编译： g++ -o main main.cpp apple.cpp
//结果
take func 1
2
take func 10
take func 100
```

|                                                      | const object(data members 不得变动) | non-const object(data members可以变动 |
| :--------------------------------------------------: | :---------------------------------: | :-----------------------------------: |
|    const member functions(保证不修改data members)    |                  T                  |                   T                   |
| non-const member functions(不保证不修改data members) |                  F                  |                   T                   |

**当成员函数的const和non-const版本同时存在，const object只能调用 const 版本 non-const object只能调用 non-const版本**

# static 那些事
当与不同类型一起使用时，Static关键字具有不同的含义。我们可以使用static关键字：

静态变量： 函数中的变量，类中的变量

静态类的成员： 类对象和类中的函数

现在让我们详细看一下静态的这些用法：

## 静态变量

### 函数中的静态变量

当变量声明为static时，空间将在程序的生命周期内分配。**即使多次调用该函数，静态变量的空间也只分配一次，前一次调用中的变量值通过下一次函数调用传递**。这对于在C / C ++或需要存储先前函数状态的任何其他应用程序非常有用。

``` c++
#include <iostream> 
#include <string> 
using namespace std; 

void demo() 
{ 
    // static variable 
    static int count = 0; 
    cout << count << " "; 

    // value is updated and 
    // will be carried to next 
    // function calls 
    count++; 
} 

int main() 
{ 
    for (int i=0; i<5; i++)
        demo(); 
    return 0; 
} 
```

输出：

``` c++
0 1 2 3 4 
```

您可以在上面的程序中看到变量count被声明为static。因此，它的值通过函数调用来传递。每次调用函数时，都不会对变量计数进行初始化。

### 类中的静态变量

由于声明为static的变量只被初始化一次，因为它们在单独的静态存储中分配了空间，因此类中的静态变量由对象共享。对于不同的对象，不能有相同静态变量的多个副本。也是因为这个原因，静态变量不能使用构造函数初始化。

``` c++
#include <iostream>
using namespace std;

class apple {
public:
  static int i;

  apple(){
      // do nothing
  };
};

int apple::i = 1;

int main() {
  apple obj1;
  apple obj2;

  cout << apple::i << endl;

  obj1.i = 2;
  cout << obj1.i << endl;

  obj2.i = 3;
  cout << obj2.i << endl;
}

```


## 静态类的成员变量

### 类对象为静态
就像变量一样，对象也在声明为static时具有范围，直到程序的生命周期。

考虑以下程序，其中对象是非静态的。

``` c++
#include<iostream> 
using namespace std; 

class Apple 
{ 
    int i; 
    public: 
        Apple() 
        { 
            i = 0; 
            cout << "Inside Constructor\n"; 
        } 
        ~Apple() 
        { 
            cout << "Inside Destructor\n"; 
        } 
}; 

int main() 
{ 
    int x = 0; 
    if (x==0) 
    { 
        Apple obj; 
    } 
    cout << "End of main\n"; 
}
```

 
输出：

``` c++
Inside Constructor
Inside Destructor
End of main
```

在上面的程序中，对象在if块内声明为非静态。因此，变量的范围仅在if块内。因此，当创建对象时，将调用构造函数，并且在if块的控制权越过析构函数的同时调用，因为对象的范围仅在声明它的if块内。 如果我们将对象声明为静态，现在让我们看看输出的变化。

``` c++
#include<iostream> 
using namespace std; 

class Apple 
{ 
    int i; 
    public: 
        Apple() 
        { 
            i = 0; 
            cout << "Inside Constructor\n"; 
        } 
        ~Apple() 
        { 
            cout << "Inside Destructor\n"; 
        } 
}; 

int main() 
{ 
    int x = 0; 
    if (x==0) 
    { 
        static Apple obj; 
    } 
    cout << "End of main\n"; 
}
```

 
输出：

``` c++
Inside Constructor
End of main
Inside Destructor
```

您可以清楚地看到输出的变化。现在，在main结束后调用析构函数。这是因为静态对象的范围是贯穿程序的生命周期。

### 类中的静态函数
就像类中的静态数据成员或静态变量一样，静态成员函数也不依赖于类的对象。我们被允许使用对象和'.'来调用静态成员函数。但建议使用类名和范围解析运算符调用静态成员。

允许静态成员函数仅访问静态数据成员或其他静态成员函数，它们无法访问类的非静态数据成员或成员函数。

``` c++
#include<iostream> 
using namespace std; 

class Apple 
{ 
    public: 
        // static member function 
        static void printMsg() 
        {
            cout<<"Welcome to Apple!"; 
        }
}; 

// main function 
int main() 
{ 
    // invoking a static member function 
    Apple::printMsg(); 
} 
```

# this指针那些时

首先来谈谈this指针的用处：

- （1）**一个对象的this指针并不是对象本身的一部分，不会影响sizeof(对象)的结果。**
- （2）this作用域是在类内部，当在类的非静态成员函数中访问类的非静态成员的时候，编译器会自动将对象本身的地址作为一个隐含参数传递给函数。也就是说，即使你没有写上this指针，编译器在编译的时候也是加上this的，**它作为非静态成员函数的隐含形参，对各成员的访问均通过this进行。**

其次，this指针的使用：

- （1）在类的非静态成员函数中返回类对象本身的时候，直接使用 return *this。
- （2）当参数与成员变量名相同时，如this->n = n （不能写成n = n)。

**总结：this在成员函数的开始执行前构造，在成员的执行结束后清除。上述的get_age函数会被解析成get_age(const A * const this),add_age函数会被解析成add_age(A* const this,int a)。在C++中类和结构是只有一个区别的：类的成员默认是private，而结构是public。this是类的指针，如果换成结构，那this就是结构的指针了。**

# inline 那些事
## 类中内联

``` c++
class A
{
public:
    void f1(int x); 

    /**
     * @brief 类中定义了的函数是隐式内联函数,声明要想成为内联函数，必须在实现处(定义处)加inline关键字。
     *
     * @param x
     * @param y
     */
    void Foo(int x,int y) ///< 定义即隐式内联函数！
    {

    };
    void f1(int x); ///< 声明后，要想成为内联函数，必须在定义处加inline关键字。  
};
```


``` c++
#include <iostream>
#include "inline.h"

using namespace std;

/**
 * @brief inline要起作用,inline要与函数定义放在一起,inline是一种“用于实现的关键字,而不是用于声明的关键字”
 *
 * @param x
 * @param y
 *
 * @return 
 */
int Foo(int x,int y);  // 函数声明
inline int Foo(int x,int y) // 函数定义
{
    return x+y;
}

// 定义处加inline关键字，推荐这种写法！
inline void A::f1(int x){

}

int main()
{
    cout<<Foo(1,2)<<endl;
}
/**
 * 编译器对 inline 函数的处理步骤
 * 将 inline 函数体复制到 inline 函数调用点处；
 * 为所用 inline 函数中的局部变量分配内存空间；
 * 将 inline 函数的的输入参数和返回值映射到调用方法的局部变量空间中；
 * 如果 inline 函数有多个返回点，将其转变为 inline 函数代码块末尾的分支（使用 GOTO）。
 */
```


> 编译器对 inline 函数的处理步骤

1. 将 inline 函数体复制到 inline 函数调用点处；
2. 为所有 inline 函数中的局部变量分配内存空间；
3. 将 inline 函数的的输入参数和返回值映射到调用方法的局部变量空间中；
4. 如果 inline 函数有多个返回点，将其转变为 inline 函数代码块末尾的分支（使用 GOTO）。

内联能提高函数效率，但并不是所有的函数都定义成内联函数！内联是以代码膨胀(复制)为代价，仅仅省去了函数调用的开销，从而提高函数的执行效率。

- 如果执行函数体内代码的时间相比于函数调用的开销较大，那么效率的收货会更少！
- 另一方面，每一处内联函数的调用都要复制代码，将使程序的总代码量增大，消耗更多的内存空间。

> 以下情况不宜用内联

- 如果函数体内的代码比较长，使得内联将导致内存消耗代价比较高。
- 如果函数体内出现循环，那么执行函数体内代码的时间要比函数调用的开销大。

## 虚函数（virtual）可以是内联函数（inline）吗？
- 虚函数可以是内联函数，内联是可以修饰虚函数的，但是当虚函数表现多态性的时候不能内联。
- 内联是在编译器建议编译器内联，而虚函数的多态性在运行期，编译器无法知道运行期调用哪个代码，因此虚函数表现为多态性时（运行期）不可以内联。
- inline virtual 唯一可以内联的时候是：编译器知道所调用的对象是哪个类（如 Base::who()），这只有在编译器具有实际对象而不是对象的指针或引用时才会发生。

``` c++
#include <iostream>
#include <istream>

using namespace std;

class Base
{
public:
  inline virtual void who() {
    cout << "I am Base\n";
  }
  virtual ~Base() {
    cout << "调用~Base()\n";
  }
};

class Derived : public Base {
public:
  inline void who() { // 不写inline时隐式内联
    cout << "I'm Derived\n";
  }
  ~Derived() {
    cout << "调用~Derived()\n";
  }
};

int main() {
  // 此处的虚函数 who() 是通过类(base) 的具体对象(b) 来调用的 编译期间就能确定了
  // 所以它可以是内联的，但最终是否内联取决于编译器。
  Base b; // 构造对象
  b.who();

  // 此处的虚函数事通过指针调用的，呈现多态性，需要运行期间才能确定，所以不能为内联
  Base *ptr = new Derived();
  ptr->who();

  // 因为Base有虚析构函数（virtual ~Base() {}），所以 delete
  // 时，会先调用派生类（Derived）析构函数，再调用基类（Base）析构函数，防止内存泄漏。
  delete ptr;
  ptr = nullptr;

  return 0;
}

```

# sizeof那些事
- 空类的大小为 1 byte
- 一个类中，虚函数本身、成员函数(包括静态与非静态)和静态数据成员都是不占用类对象的存储空间的
- 对于包含虚函数的类，不管有多少个虚函数，只有一个虚指针，vptr的大小
- 普通继承，派生类继承了所有积累的函数与成员，要按照字节对齐来计算大小
- 虚函数继承，不管是单继承还是多继承，都是继承了基类的vptr
- 虚继承 继承基类的vptr

``` c++
#include <iostream>

using namespace std;

class void_class {};

class non_void_class {
private:
  int da;
  void_class db;
  long dc;

public:
};

class Base {
public:
  virtual void vfunc() { cout << "this is Base.vfunc()\n"; }

  void who() { cout << "this is Base.who()\n"; }

private:
  static int a;
  static void_class b;
  static long c;
};

class VBase {
public:
  virtual void vfunc1() { cout << "this is VBase.vfunc1()\n"; }

  virtual void vfunc2() { cout << "this is VBase.vfunc2()\n"; }

  virtual void who() { cout << "this is Base.who()\n"; }

private:
  static int a;
  static void_class b;
  static long c;
};

class Derive : public Base {
private:
  int da;
  void_class db;
  long dc;

public:
};

class VDerive : public Base, public VBase {
private:
  int da;
  void_class db;
  long dc;

public:
  virtual void name() { cout << "I am VDerive\n"; }
};

int main() {
  void_class v;
  cout << "空类 " << sizeof(v) << endl;

  Base b;
  cout << "有虚函数 静态数据 " << sizeof(b) << endl; // 虚指针的大小

  non_void_class nv;
  cout << "非空类 有非静态数据 无虚函数 无继承 " << sizeof(nv) << endl;

  Derive d;
  cout << "单继承(父类有虚函数) 有非静态数据 " << sizeof(d) << endl;

  VDerive vd;
  cout << "多继承(父类有多个虚函数) 有非静态数据" << sizeof(vd) << endl;
}

```


# 纯虚函数和抽象类那些事


# vptr vtable那些事


# virtual那些事


# volatile那些事


# assert那些事


# 位域那些事


# extern那些事


# struct那些事


# struct和class那些事


# union那些事



# C实现C++多态那些事



# explicit那些事



# friend那些事



# using那些事



# ::那些事



# enum那些事



# decltype那些事



# 引用与指针那些事


# 宏那些事
