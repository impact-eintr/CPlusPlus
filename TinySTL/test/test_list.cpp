/*************************************************************************
    > File Name: test_list.cpp
    > Author: Stewie
    > E-mail: 793377164@qq.com
    > Created Time: 2018-05-25
*************************************************************************/
#include <list>

#include "../list.h"
#include "profiler.h"
#include "test_util.h"

template <class T>
using stdList = std::list<T>;

template <class T>
using myList = STL::list<T>;

template <class Container>
void init_Widget_list(Container& l)
{
    l.clear();
    for (int i = 1; i <= 4; ++i) {
        l.emplace_back(i * 0.5, string(i, i + '0'));
    }
}

// 构造/拷贝构造/移动构造/赋值函数、拷贝/移动赋值操作符
void test_case1()
{
    cout << "<test_case01>" << endl;
    
    myList<Widget> l0;
    myList<Widget> l1(10);              
    myList<Widget> l2(10, {123, "ysw"});
    Print(l0, l1, l2);
    
    myList<string> l3{"aaa", "bbb", "ccc", "ddd"};
    myList<string> l4(l3.begin(), l3.end());
    Print(l3, l4);
    assert(Container_Equal(l3, l4));

    myList<string> l5(l3);
    myList<string> l6(l4);
    assert(Container_Equal(l5, l6));

    myList<string> l7(std::move(l3));
    myList<string> l8(std::move(l4));
    Print(l3, l4);
    assert(Container_Equal(l7, l8));

    l3 = l7;              
    l4 = std::move(l8);   
    Print(l3, l4);
    assert(Container_Equal(l3, l4));
}

// 元素访问
void test_case2()
{
    cout << "<test_case02>" << endl;

    myList<Widget> l1;
    stdList<Widget> l2;
    init_Widget_list(l1); 
    init_Widget_list(l2); 
    Print(l1, l2);
    assert(Container_Equal(l1, l2));  
    
    l1.front() = l2.front() = {12.3, "aaa"};
    l1.back() = l2.back() = {0, "ysw"};
    Print(l1, l2);
    assert(Container_Equal(l1, l2));  
}

// 迭代器
void test_case3()
{
    cout << "<test_case03>" << endl;

    myList<int> l;
    int i;
    for (i = 0; i < 1000; ++i)
        l.push_back(i);
    
    i = 0;
    for (myList<int>::iterator it = l.begin(); it != l.end(); ++it, ++i) {
        assert(*it == i);
        *it = i * 2;
    }
    
    --i;
    for (myList<int>::const_iterator cit = --l.cend(); ; --cit, --i) {
        assert(*cit == i * 2);
        if (cit == l.cbegin())  break;
    }
}

// 容量
void test_case4() 
{
    cout << "<test_case04>" << endl;

    myList<int> l(100);
    assert(l.size() == 100);
    
    l.resize(5);
    assert(l.size() == 5);
    l.resize(50);
    assert(l.size() == 50);

    l.clear();
    assert(l.empty());
    assert(l.size() == 0);
}

// push_back()、emplace_back()
// push_front()、emplace_front()
// pop_back()、pop_front()
void test_case5()
{
    cout << "<test_case05>" << endl;

    myList<Widget> l1;
    stdList<Widget> l2;
    init_Widget_list(l1);
    init_Widget_list(l2);
    Print(l1, l2);
    assert(Container_Equal(l1, l2));

    l1.push_back({12.3, "aaa"});    l1.emplace_back(45.6, "bbb"); 
    l2.push_back({12.3, "aaa"});    l2.emplace_back(45.6, "bbb");
    l1.push_front({78.9, "ccc"});   l1.emplace_front(0, "ysw");
    l2.push_front({78.9, "ccc"});   l2.emplace_front(0, "ysw");
    Print(l1, l2);
    l1.pop_back();  l1.pop_front();
    l2.pop_back();  l2.pop_front();
    Print(l1, l2);
    assert(Container_Equal(l1, l2));
}

// insert()、erase()
void test_case6()
{
    cout << "<test_case06>" << endl;

    myList<int> l1, l3;
    stdList<int> l2, l4;
    for (int i = 1; i <= 1000; ++i) {
        l3.push_back(i);
        l4.push_back(i);
    }
    assert(Container_Equal(l3, l4));

    // 3种insert()
    l1.insert(l1.end(), 1);     l1.insert(l1.end(), 1, 1000);
    l2.insert(l2.end(), 1);     l2.insert(l2.end(), 1, 1000);
    l1.insert(--l1.end(), ++l3.begin(), --l3.end());
    l2.insert(--l2.end(), ++l4.begin(), --l4.end());
    assert(Container_Equal(l1, l2));
    assert(Container_Equal(l1, l3));

    // 2种erase
    l1.erase(l1.begin());
    l2.erase(l2.begin());
    l1.erase(++l1.begin(), --l1.end());
    l2.erase(++l2.begin(), --l2.end());
    Print(l1, l2); 
    assert(Container_Equal(l1, l2));
}

// merge()
void test_case7() 
{
    cout << "<test_case07>" << endl;

    myList<int> l1{1, 3, 5, 7, 9}, l2{0, 2, 4, 6, 8};
    Print(l1, l2);
    l1.merge(l2);
    Print(l1, l2);
}

// splice()
void test_case8()
{
    cout << "<test_case08>" << endl;

    myList<int> l1, l3;
    stdList<int> l2, l4;
    for (int i = 1; i <= 1000; ++i) {
        l3.push_back(i);
        l4.push_back(i);
    }
    assert(Container_Equal(l3, l4));

    for (int i = 999; i > 500; --i) {
        auto it3 = l3.begin(); 
        auto it4 = l4.begin(); 
        int n = get_rand(0, i);
        STL::advance(it3, n);
        std::advance(it4, n);
        l1.splice(l1.end(), l3, it3);
        l2.splice(l2.end(), l4, it4);
    }
    assert(Container_Equal(l1, l2));
    l1.splice(l1.end(), l3, l3.begin(), l3.end());
    l2.splice(l2.end(), l4, l4.begin(), l4.end());
    assert(Container_Equal(l1, l2));
}

// remove
void test_case9()
{
    cout << "<test_case09>" << endl;

    myList<int> l1{1, 2, 3, 4, 5};
    myList<int> l2(l1);
    for (int i = 0; i < 1000; ++i)
        l2.insert(++l2.begin(), 999);
    assert(l2.size() == 1005);

    l2.remove(999);
    assert(Container_Equal(l1, l2));
}

// unique
void test_case10()
{
    cout << "<test_case10>" << endl;

    myList<int> l{1, 2, 3, 3, 3, 3, 2, 1, 4, 4, 4, 5};
    Print(l);
    l.unique();
    Print(l);
}

// sort()、reverse()
void test_case11()
{
    cout << "<test_case11>" << endl;

    myList<int> l1, l3;
    stdList<int> l2, l4;
    for (int i = 1; i <= 1000; ++i) {
        l3.push_back(i);
        l4.push_back(i);
    }
    assert(Container_Equal(l3, l4));

    for (int i = 1; i <= 1000; ++i) {
        auto it3 = l3.begin();  int n3 = get_rand(0, l3.size() - 1);
        STL::advance(it3, n3);
        auto it4 = l4.begin();  int n4 = get_rand(0, l4.size() - 1);
        std::advance(it4, n4);
        l1.splice(l1.end(), l3, it3);
        l2.splice(l2.end(), l4, it4);
    }
    assert(!Container_Equal(l1, l2));
    
    l1.sort();
    l2.sort();
    assert(Container_Equal(l1, l2));
    
    l1.reverse();
    l2.reverse();
    assert(Container_Equal(l1, l2));
}

// 比较符
void test_case12()
{
    cout << "<test_case12>" << endl;

    myList<Widget> l1, l2, l3;
    init_Widget_list(l1); 
    init_Widget_list(l2); 
    Print(l1, l2);
    assert(l1 == l2);
    assert(l1 != l3);
}

void test_all_cases()
{
    test_case1();
    test_case2();
    test_case3();
    test_case4();
    test_case5();
    test_case6();
    test_case7();
    test_case8();
    test_case9();
    test_case10();
    test_case11();
    test_case12();
}

// 性能测试
void test_performance()
{
    cout << "<test_performance>" << endl;
    const int max = 1000000;

    myList<int> l1;
    Profiler::Start();
    for (int i = 0; i < max * 10; ++i)
        l1.push_back(i);
    Profiler::Finish();
    Profiler::dumpDuration();
    
    stdList<int> l2;
    Profiler::Start();
    for (int i = 0; i < max * 10; ++i)
        l2.push_back(i);
    Profiler::Finish();
    Profiler::dumpDuration();

    l1.clear();
    l2.clear();

    for (int i = 0; i != max; ++i) {
        int n = get_rand(-max, max);
        l1.push_back(n);
        l2.push_back(n);    
    }

    Profiler::Start();
    l1.sort();
    Profiler::Finish();
    Profiler::dumpDuration();
    
    Profiler::Start();
    l2.sort();
    Profiler::Finish();
    Profiler::dumpDuration();

}

int main()
{
    test_all_cases();
    test_performance();
    return 0;
}
