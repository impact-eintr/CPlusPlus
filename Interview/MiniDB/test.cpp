#include "minidb.h"
#include <iostream>
#include <memory>

using namespace std;

int main() {
  shared_ptr<minidb> db(minidb_Factor().get("/tmp")); // 建议使用智能指针
  // 原生指针 记得 delete db; 这个bug调了半天 还以为虚析构函数出问题了
  //minidb *db = minidb_Factor().get("/tmp");
  cout << db->get("k1") << endl;
  cout << db->get("k2") << endl;
  for (int i = 0;i < 1000000;++i) {
    db->put("k"+to_string(i), "v"+to_string(i));
  }
  cout << db->get("k3") << endl;
  cout << db->get("k4") << endl;
  db->del("k3");
  db->del("k4");
  cout << db->get("k3") << endl;
  cout << db->get("k4") << endl;
}
