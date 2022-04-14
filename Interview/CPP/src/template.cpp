#include <iostream>
#include <algorithm>
#include <string>

using std::cout;
using std::endl;
using std::string;
using std::max;

// 模拟tuple内部实现
template <typename... Values> class _tuple; // 这里表示_tuple是一个模板类

template <> class _tuple<> {
public:
  _tuple() {}
};

template <typename Head, typename... Tail> //不希望产生实参个数为0的变长参数模板
class _tuple<Head, Tail...> : private _tuple<Tail...> //继承自参数少一个的tuple
{
  typedef _tuple<Tail...> inherited;

protected:
  Head m_head;

public:
  _tuple() = default;
  _tuple(Head v, Tail... vtail) : m_head(v), inherited(vtail...) {}

  auto head() -> decltype(m_head) { return m_head; } //使用decltype进行类型推导
  //	Head head(){ return m_head; }
  inherited &tail() { return *this; } // return后转换为inherited
};
// Max/Min
int maximum(int n) { return n; }

template <typename... Args> int maximum(int n, Args... args) {
  return max(n, maximum(args...));
}

int main(int argc, char *argv[]) {
  _tuple<int,float,string> t(41,6.3,"nico");
  cout << t.head() <<endl;
  cout << t.tail().head() <<endl;
  cout << t.tail().tail().head() <<endl;

  cout << maximum(2, 1, 3) << endl;

  return 0;
}
