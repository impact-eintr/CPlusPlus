#include <iostream>
#include <typeinfo>

using namespace std;



class B {
  virtual void func() {}
};

class D : public B {};

int main() {
  B *b = new D; // 向上转型
  B &obj = *b;
  D *d = dynamic_cast<D *>(b); // 向下转型
  if (d != nullptr) {
    cout << "cast B* to D*\n";
  } else {
    cout << "cannot cast B* to D*\n";
  }

  try {
    D &dobj = dynamic_cast<D &>(obj);
    cout << "works!\n";
  } catch (bad_cast bc) {
    cout << bc.what() << endl;
  }
  return 0;
}
