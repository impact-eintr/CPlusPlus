# C++STL体系结构与内核分析
C++标准库 > STL

> 先感受一下

``` c++
#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>

using namespace std;

int main(int argc, char *argv[]) {

  int ia[6] = {27, 210, 12, 47, 109, 83};
  vector<int, allocator<int>> vi(ia, ia+6);

  cout << count_if(vi.begin(), vi.end(), not1(bind2nd(less<int>(), 40)));

  return 0;
}

```


## 容器
- Array
- Vector
- Dqueue
- List
- Forward-List

- Set/Multiset
- Map/Multimap



``` c++
#include <array>
#include <cstdlib>
#include <ctime>
#include <string>
#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>

using namespace std;

const long ASIZE = 10000 *50;

namespace util{
  long get_a_target_long() {
    long target = 0;
    cout << "target (0~" << RAND_MAX << "): ";
    cin >> target;
    return target;
  }

  string get_a_target_string() {
    long target = 0;
    char buf[10];

    cout << "target (0~" << RAND_MAX << "): ";
    cin >> target;
    snprintf(buf, 10, "%ld", target);
    return string(buf);
  }

  int compareLongs(const void* a, const void* b) {
    return (*(long*)a - *(long*)b);
  }

  int compareStrings(const void* a,const void* b) {
    if (*(string*)a > *(string*)b) {
      return 1;
    } else if (*(string*)a < *(string*)b) {
      return -1;
    } else {
      return 0;
    }
  }
}

namespace container {
  void test_array() {
    cout << "test_array()...........\n";
    array<long, ASIZE> c;

    clock_t timeStart = clock();
    for (long i = 0;i < ASIZE;i++) {
      c[i] = rand();
    }
    cout << "milli-seconds: " << (clock() - timeStart) << endl;
    cout << "array.size(): " << c.size() << endl;
    cout << "array.front(): " << c.front() << endl;
    cout << "array.back(): " << c.back() << endl;
    cout << "array.data(): "  << c.data() << endl;

    long target = util::get_a_target_long();
    timeStart = clock();
    qsort(c.data(), ASIZE, sizeof(long), util::compareLongs);
    long* pItem = (long*)bsearch(&target, (c.data()), ASIZE, sizeof(long), util::compareLongs);

    cout << "qsort() + bsearch(), milli-seconds : " << (clock() - timeStart) << endl;

    if (pItem != nullptr) {
      cout << "found, " << *pItem << endl;
    } else {
      cout << "not found!" << endl;
    }
  }
}

int main(int argc, char *argv[]) {

  container::test_array();
  return 0;
}

```


## 分配器




## 算法




## 迭代器
前闭后开 `[ )`

``` c++
Container<T> c;
...
Container<T> it = c.begin();
for (; it != c.end();++it){
    // 
}
```

``` c++
std::vector<double> vec;
...
for (auto elem : vec) {
  std::cout << elem << std::endl;
}

...
for (auto& elem : vec) {
  elem *= 3;
}
```




## 适配器




## 仿函数


