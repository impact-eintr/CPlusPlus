#ifndef TINYSTL_TEST_UTIL_H_
#define TINYSTL_TEST_UTIL_H_ 

#include <iostream>
#include <functional>
#include <string>
#include <assert.h>
#include <ctime>
#include <random>

using std::cout;
using std::endl;
using std::ostream;
using std::string;

// 非POD类型struct
struct Widget
{
    double d = 0.0;
    string s = string();

    Widget() { }
    Widget(double _d, string _s) : d(_d), s(_s) { }
    Widget(const Widget& x) : d(x.d), s(x.s) { }

    bool operator==(const Widget& x) const
    { return d == x.d && s == x.s; }

    bool operator!=(const Widget& x) const
    { return !(*this == x); }

    bool operator<(const Widget& x) const
    { return d < x.d || (d == x.d && s < x.s); }
};

// Widget类输出重载
ostream& operator<<(ostream& os, const Widget& x)
{
    os << '(' << x.d << ' ' << x.s << ')';
    return os;
}

// lambda
auto WidgetLess = [](const Widget& lhs, const Widget& rhs) { 
    return lhs.d < rhs.d || (lhs.d == rhs.d && lhs.s < rhs.s); };

auto WidgetEqual = [](const Widget& lhs, const Widget& rhs) {
    return lhs.d == rhs.d && lhs.s == rhs.s; };

// 容器输出函数
template <class Container>
void Print(const Container& c)
{
    printf("size: %02zd  { ", c.size());
    for (auto it = c.begin(); it != c.end(); ++it) {
        cout << *it << ' ';
    }
    cout << '}' << endl;
}

template <class Container, class... Args>
void Print(const Container& c, Args... args)
{
    Print(c);
    if (sizeof...(Args))
        Print(args...);
}

// 判断容器相等
template <class Container1, class Container2>
bool Container_Equal(const Container1& c1, const Container2& c2)
{
    auto first1 = c1.begin(), last1 = c1.end();
    auto first2 = c2.begin(), last2 = c2.end();
    for ( ; first1 != last1 && first2 != last2; ++first1, ++first2) {
        if (*first1 != *first2)
            return false;
    }
    return first1 == last1 && first2 == last2;
}

// 获得[a, b]的随机整数
inline int get_rand(int a, int b)
{
    std::random_device rd;
    std::default_random_engine engine(rd());
    std::uniform_int_distribution<int> dis(a, b); 
    auto dice = std::bind(dis, engine); // <functional>
    return dice();
}

#endif
