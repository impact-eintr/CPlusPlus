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
