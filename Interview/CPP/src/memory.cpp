#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;
class A
{
private:
  char *p;
  unsigned int p_size;
  int *p_count; // 计数变量
public:
  A(unsigned int n = 1) {// 在构造函数中申请内存
    cout << "构造" << endl;
    p = new char[n];
    p_size = n;
    p_count = new int;
    *p_count = 1;
    cout << "count is : " << *p_count << endl;
  }
  A(const A &temp){
    cout << "拷贝构造" << endl;
    p = temp.p;
    p_size = temp.p_size;
    p_count = temp.p_count;
    (*p_count)++; // 复制时，计数变量 +1
    cout << "count is : " << *p_count << endl;
  }
  A operator=(const A &temp) {
    if (*this == temp) {
      return;
    }
  }
  ~A()
    {
      (*p_count)--; // 析构时，计数变量 -1
      cout << "count is : " << *p_count << endl;

      if (*p_count == 0) // 只有当计数变量为 0 的时候才会释放该块内存空间
      {
        cout << "buf is deleted" << endl;
        if (p != NULL)
        {
          delete[] p; // 删除字符数组
          p = NULL;   // 防止出现野指针
          if (p_count != NULL)
          {
            delete p_count;
            p_count = NULL;
          }
        }
      }
    };
  char *GetPointer()
    {
      return p;
    };
};
void fun()
{
  A ex(100);
  char *p = ex.GetPointer();
  strcpy(p, "Test");
  cout << p << endl;

  A ex1 = ex; // 此时计数变量会 +1
  cout << "ex1.p = " << ex1.GetPointer() << endl;
}
int main()
{
  fun();
  return 0;
}
