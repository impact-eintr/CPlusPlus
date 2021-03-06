## 深入理解左值和右值

``` c++
void fun(int &x) {
  // DO 
}

int main() {
  fun(10);
  return 0;
}
```

其中上述报错中的rvalue就是10，也就是说10就是rvalue，那么到底什么是rvalue，rvalue的意义是什么？这就是本文的目的，通过本文，让你彻底搞清楚什么C++下的值类别，以及如何区分左值、纯右值和将亡值。

C语言中的左值与CPL中的左值区别是，在C语言中lvalue是locator value的简写，因此lvalue对应了一块内存地址。

C++11之前，左值遵循了C语言的分类法，但与C不同的是，其将非左值表达式统称为右值，函数为左值，并添加了引用能绑定到左值但唯有const的引用能绑定到右值的规则。几种非左值的C表达式在C++中成为了左值表达式

自C++11开始，对值类别又进行了详细分类，在原有左值的基础上增加了纯右值和消亡值，并对以上三种类型通过是否具名(identity)和可移动(moveable)，又增加了glvalue和rvalue两种组合类型，在后面的内容中，会对这几种类型进行详细讲解。

### 表达式

C/C++代码是由标识符、表达式和语句以及一些必要的符号(大括号等)组成。

表达式由按照语言规则排列的运算符，常量和变量组成。一个表达式可以包含一个或多个操作数，零个或多个运算符来计算值。每个表达式都会产生一些值，该值将在赋值运算符的帮助下分配给变量。

在C/C++中，表达式有很多种，我们常见的有前后缀表达式、条件运算符表达式等。字面值(literal)和变量(variable)是最简单的表达式，函数的返回值也被认为是表达式。

表达式是可求值的，对表达式求值可得到一个结果，这个结果有两个属性:

- 类型。这个我们很常见，比如int、string、引用或者我们自定义的类。类型确定了表达式可以进行哪些操作。
- 值类别(在下节中会细讲)。

### 值类别
在上节中，我们提到表达式是可求值的，而值类别就是求值结果的属性之一。

在C++11之前，表达式的值分为左值和右值两种，其中右值就是我们理解中的字面值1、true、NULL等。

自C++11开始，表达式的值分为`左值(lvalue, left value)`、`将亡值(xvalue, expiring value)`、`纯右值(pvalue, pure ravlue)`以及两种混合类别`泛左值(glvalue, generalized lvalue)`和`右值(rvalue, right value)`五种。

这五种类别的分类基于表达式的两个特征：

- 具名(identity):可以确定表达式是否与另一表达式指代同一实体，例如通过比较它们所标识的对象或函数的（直接或间接获得的）地址
- 可被移动：移动构造函数、移动赋值运算符或实现了移动语义的其他函数重载能够绑定于这个表达式

结合上述两个特征，对五种表达式值类别进行重新定义：
- lvalue:具名且不可被移动
- xvaue:具名且可被移动
- prvalue:不具名且可被移动
- glvalue:具名，lvalue和xvalue都属于glvalue
- rvalue:可被移动的表达式，prvalue和xvalue都属于rvalue

### 左值
左值(lvalue,left value)，顾名思义就是赋值符号左边的值。**准确来说，左值是表达式结束（不一定是赋值表达式）后依然存在的对象。**

可以将左值看作是一个关联了名称的内存位置，允许程序的其他部分来访问它。在这里，我们将 "名称" 解释为任何可用于访问内存位置的表达式。所以，如果 arr 是一个数组，那么 arr[1] 和 *(arr+1) 都将被视为相同内存位置的“名称”。

左值具有以下特征：
- 可通过取地址运算符获取其地址
- 可修改的左值可用作内建赋值和内建复合赋值运算符的左操作数
- 可以用来初始化左值引用(后面有讲)

> 那么哪些都是左值呢?查了相关资料，做了些汇总，基本覆盖了所有的类型：

- 变量名、函数名以及数据成员名
- 返回左值引用的函数调用
- 由赋值运算符或复合赋值运算符连接的表达式，如(a=b, a-=b等)
- 解引用表达式*ptr
- 前置自增和自减表达式(++a, ++b)
- 成员访问（点）运算符的结果
- 由指针访问成员（ -> ）运算符的结果
- 下标运算符的结果([])
- 字符串字面值("abc")

``` c++
int a = 1; // a为左值
T& f();
f(); // 左值
++a; // 左值
--a; // 左值
int b = a; // a b 都是左值
struct S* ptr = &obj; // ptr 为左值
arr[1] = 2; // 左值
int *p = &a; // p为左值
*p = 10; // *p为左值
class MyClass{};
MyClass c; /// c为左值
"abc" // 左值
```

> **对于一个表达式，凡是对其取地址（&）操作可以成功的都是左值**

### 纯右值
在前面有提过，自C++11开始，纯右值(pvalue, pure ravlue)相当于之前的右值，那么什么是纯右值呢？

字面值或者函数返回的非引用都是纯右值。

> **以下表达式的值都是纯右值：**

- 字面值(字符串字面值除外)，例如1，'a', true等
- 返回值为非引用的函数调用或操作符重载，例如：str.substr(1, 2), str1 + str2, or it++
- 后置自增和自减表达式(a++, a--)
- 算术表达式
- 逻辑表达式
- 比较表达式
- 取地址表达式
- lambda表达式

``` c++
nullptr;
true;
1;
int fun();
fun();

int a = 1;
int b = 2;
a + b;

a++;
b--;

a > b;
a && b;
```

> 纯右值特征：

- 等同于C++11之前的右值
- 不会是多态
- 不会是抽象类型或数组
- 不会是不完全类型

### 将亡值
将亡值(xvalue, expiring value)，顾名思义即将消亡的值，是C++11新增的跟右值引用相关的表达式，通常是将要被移动的对象（移为他用），比如**返回右值引用T&&的函数返回值**、**std::move的返回值**，或者**转换为T&&的类型转换函数的返回值**。

将亡值可以理解为通过“盗取”其他变量内存空间的方式获取到的值。在确保其他变量不再被使用、或即将被销毁时，通过“盗取”的方式可以避免内存空间的释放和分配，能够延长变量值的生命期。（通过右值引用来续命）。

xvalue 只能通过两种方式来获得，这两种方式都涉及到将一个左值赋给(转化为)一个右值引用：
- 返回右值引用的函数的调用表达式,如 `static_cast<T&&>(t);` 该表达式得到一个 xvalue
- 转换为右值引用的转换函数的调用表达式，如：`std::move(t)`、`satic_cast<T&&>(t)`

``` c++
std::string fun() {
  std::string str;
  // ...
  
  return str;
}

std::string s = fun();
```

在函数fun()中，str是一个局部变量，并在函数结束时候被返回。

在C++11之前，s = fun();会调用拷贝构造函数，会将整个str复制一份，然后把str销毁。如果str特别大的话，会造成大量额外开销。在这一行中，s是左值，fun()是右值(纯右值)，fun()产生的那个返回值作为一个临时值，一旦str被s复制后，将被销毁，无法获取、也不能修改。

自C++11开始，引入了`move`语义，编译器会将这部分优化成move操作，即不再是之前的复制操作，而是move。此时，str会被进行隐式右值转换，等价于`static_cast<std::string&&>(str)`，进而此处的 s 会将 foo 局部返回的值进行移动。

> 无论是C++11之前的拷贝，还是C++11的move，str在填充(拷贝或者move)给s之后，将被销毁，而被销毁的这个值，就成为将亡值。

**将亡值就定义了这样一种行为：具名的临时值、同时又能够被move。**

### 混合类型

#### 泛左值

``` c++
struct S{
  int n;
};

S fun();
S s;
s;
std::move(s);

fun();
S{};
S{}.n;
```

- 定义了结构体S和函数fun()
- 第6行声明了类型为S的变量s，因为其是具名的，所以是glvalue
- 第七行同上，因为s具名，所以为glvalue
- 第8行中调用了move函数 ，将左值s转换成xvalue，所以是glvaue
- 第10行中，fun()是不具名的，是纯右值，所以不是glvalue
- 第11行中，生成一个不具名的临时变量，是纯右值，所以不是glvalue
- 第12行中，n具名，所以是glvalue

glvalue的特征如下：

- 可以自动转换成prvalue
- 可以是多态的
- 可以是不完整类型，如前置声明但未定义的类类型

#### 右值
右值(rvalue, right value)是指可以移动的表达式。prvalue和xvalue都是rvalue，具体的示例见下文。

rvalue具有以下特征：

- 无法对rvalue进行取地址操作。例如：&1，&(a + b)，这些表达式没有意义，也编译不过。
- rvalue不能放在赋值或者组合赋值符号的左边，例如：3 = 5，3 += 5，这些表达式没有意义，也编译不过。
- rvalue可以用来初始化const左值引用（见下文）。例如：const int& a = 1。
- rvalue可以用来初始化右值引用（见下文）。
- rvalue可以影响函数重载：当被用作函数实参且该函数有两种重载可用，其中之一接受右值引用的形参而另一个接受 const 的左值引用的形参时，右值将被绑定到右值引用的重载之上。

### 引用

既然提到了左值右值，就得提一下引用。

在C++11之前，引用分为左值引用和常量左值引用两种，但是自C++11起，引入了右值引用，也就是说，在C++11中，包含如下3中引用：
- 左值引用
- 常量左值引用（不希望被修改）
- 右值引用

``` c++
int a = 1;
int &rb = a; // b为左值引用
int &&rrb = a; // 错误，a是左值，右值引用不能绑定左值
int &&rrb1 = 1; // 正确，1为右值
int &rb1 = i * 2; // 错误，i * 2是右值，而rb1位左值引用
int &&rrb2 = i * 2; // 正确
const int &c = 1; // 正确
const int &c1 = i * 2; // 正确
```

在这里，我们需要特别注意的一点就是右值引用虽然是引用右值，但是其本身是左值，以下代码为例：

``` c++
int &a = 1;
```

在上述代码中，a是一个右值引用，但是其本身是左值，这是因为：
- a出现在等号(=)的左边
- 可以对a取地址

**我们在前面有提到过，一个表达式有两个属性，分别为类型和值类别。本节说的`左值引用和右值引用就属于类型`,而`左值和右值则属于值类别范畴`，这个概念很重要，千万不能混淆。**

### 系统API

``` c++
std::is_lvalue_reference
is_rvalue_reference
  
int a = 1;
int &ra = a;
int &&b = 1;

std::cout << std::is_lvalue_reference<decltype(ra)>::value << std::endl;

std::cout << std::is_rvalue_reference<decltype(ra)>::value << std::endl;

std::cout << std::is_rvalue_reference<decltype(b)>::value << std::endl;

```

``` c++
1
0
1
```


## 编译器之返回值优化


## 深入理解移动语义

``` c++
#include <cstdlib>
#include <iostream>
#include <vector>

class BigObj {
public:
  explicit BigObj(size_t length) noexcept: length_(length), data_(new int[length]) {
    std::cout << "构造BigObject" << std::endl;
  }

  ~BigObj() {
    if (data_ != nullptr) {
      delete[] data_;
      length_ = 0;
    }
  }

  // 拷贝构造函数
   BigObj(const BigObj &other) noexcept
      : length_(other.length_), data_(new int[other.length_]) {
    std::cout << "使用拷贝构造" << std::endl;
    std::copy(other.data_, other.data_ + length_, data_);
  }
  // 赋值运算符
  BigObj &operator=(BigObj &other) {
    std::cout << "使用赋值" << std::endl;
    if (this != &other) {
      delete[] data_;
      length_ = other.length_;
      data_ = new int[length_];
      std::copy(other.data_, other.data_ + length_, data_);
    }
    return *this;
  }
  // 移动构造函数
  // 这里要将移动构造函数声明为noexcept表示它不会抛出异常，
  // 这样vector<BigObj>在复制时就会使用移动迭代器（就是会包装一层std::move），从而触发移动构造。
  BigObj(BigObj &&other) noexcept{
    std::cout << "使用移动构造" << std::endl;
    data_ = other.data_;
    length_ = other.length_;
    other.data_ = nullptr;
    other.length_ = 0;
  }
  // 移动赋值运算符
  BigObj &operator=(BigObj &&other) {
    std::cout << "使用移动赋值" << std::endl;
    if (this != &other) {
      delete[] data_;
      data_ = other.data_;
      length_ = other.length_;
      other.data_ = nullptr;
      other.length_ = 0;
    }
    return *this;
  }

private:
  size_t length_;
  int *data_;
};

int main() {
  std::vector<BigObj> v;
  std::cout << "TEST1\n";
  v.push_back(BigObj(25));
  v.push_back(BigObj(75));

  std::cout << "TEST2\n";
  v.push_back(BigObj(25));
  v.push_back(BigObj(75));

  std::cout << "TEST3\n";
  v.push_back(BigObj(25));
  v.push_back(BigObj(25));
  v.push_back(BigObj(25));
  v.push_back(BigObj(25));

  std::cout << "TEST4\n";
  v.push_back(BigObj(75));

  //v.insert(v.begin() + 1, BigObj(50));
  return 0;
}

```

### 生成时机

众所周知，在C++中有四个特殊的成员函数：默认构造函数、析构函数，拷贝构造函数，拷贝赋值运算符。之所以称之为特殊的成员函数，这是因为如何开发人员没有定义这四个成员函数，那么编译器则在满足某些特定条件(仅在需要的时候才生成，比如某个代码使用它们但是它们没有在类中明确声明)下，自动生成。这些由编译器生成的特殊成员函数是public且inline。

自C++11起，引入了另外两只特殊的成员函数：移动构造函数和移动赋值运算符。如果开发人员没有显示定义移动构造函数和移动赋值运算符，那么编译器也会生成默认。与其他四个特殊成员函数不同，编译器生成默认的移动构造函数和移动赋值运算符需要，满足以下条件：

- 如果一个类定义了自己的拷贝构造函数,拷贝赋值运算符或者析构函数(这三者之一，表示程序员要自己处理对象的复制或释放问题)，编译器就不会为它生成默认的移动构造函数或者移动赋值运算符，这样做的目的是防止编译器生成的默认移动构造函数或者移动赋值运算符不是开发人员想要的
- 如果类中没有提供移动构造函数和移动赋值运算符，且编译器不会生成默认的，那么我们在代码中通过std::move()调用的移动构造或者移动赋值的行为将被转换为调用拷贝构造或者赋值运算符
- 只有一个类没有显示定义拷贝构造函数、赋值运算符以及析构函数，且类的每个非静态成员都可以移动时，编译器才会生成默认的移动构造函数或者移动赋值运算符
- 如果显式声明了移动构造函数或移动赋值运算符，则拷贝构造函数和拷贝赋值运算符将被 隐式删除（因此程开发人员必须在需要时实现拷贝构造函数和拷贝赋值运算符）
与拷贝操作一样，如果开发人员定义了移动操作，那么编译器就不会生成默认的移动操作，但是编译器生成移动操作的行为和生成拷贝操作的行为有些许不同，如下：

两个拷贝操作是独立的：声明一个不会限制编译器生成另一个。所以如果你声明一个拷贝构造函数，但是没有声明拷贝赋值运算符，如果写的代码用到了拷贝赋值，编译器会帮助你生成拷贝赋值运算符。同样的，如果你声明拷贝赋值运算符但是没有拷贝构造函数，代码用到拷贝构造函数时编译器就会生成它。上述规则在C++98和C++11中都成立。
两个移动操作不是相互独立的。如果你声明了其中一个，编译器就不再生成另一个。如果你给类声明了，比如，一个移动构造函数，就表明对于移动操作应怎样实现，与编译器应生成的默认逐成员移动有些区别。如果逐成员移动构造有些问题，那么逐成员移动赋值同样也可能有问题。所以声明移动构造函数阻止编译器生成移动赋值运算符，声明移动赋值运算符同样阻止编译器生成移动构造函数。

## 内存泄露

## 智能指针

