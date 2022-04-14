#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <functional>

using std::map;
using std::string;
using std::function;

struct adder {
  adder(int n) :_n(n) {}
  int operator() (int x) const {
    return x + _n;
  }
private:
  int _n;
};

int main() {
  auto add_2 = adder(2);

  adder add_3(3);

  std::cout << add_2(20) << " " << add_3(30) << std::endl;

  map<string, function<int(int, int)>> op_dict{
    {"+", [](int x, int y){
      return x + y;
    }},
    {"-", [](int x, int y){
      return x - y;
    }},
    {"*", [](int x, int y){
      return x * y;
    }},
    {"/", [](int x, int y){
      return x / y;
    }},
  };

  std::cout << op_dict["+"](add_2(20), add_3(30)) << std::endl;


  auto adder = [](int n) {
    return [n] (int x) {
      return x + n;
    };
  };
}
