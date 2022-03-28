#include "minidb.h"
#include <iostream>
#include <memory>
#include <thread>

using namespace std;

void race() {
  shared_ptr<minidb> db_ptr(minidb_Factor().get("/tmp"));
  thread thr1([db_ptr]() {
    for (int i = 0; i < 100000; ++i) {
      //cout << db_ptr->get("k" + to_string(i)) << endl;
      db_ptr->get("k" + to_string(i));
    }
  });
  //thread thr2([db_ptr]() {
  //  for (int i = 50000; i < 10000; ++i) {
  //    db_ptr->put("k" + to_string(i), "v" + to_string(i));
  //  }
  //});
  //thread thr3([db_ptr]() {
  //  for (int i = 0; i < 50000; ++i) {
  //    db_ptr->put("k" + to_string(i), "v" + to_string(i));
  //  }
  //});

  thr1.join();
  //thr2.join();
  //thr3.join();
}

void KV_test() {
  shared_ptr<minidb> db(minidb_Factor().get("/tmp")); // 建议使用智能指针
  cout << db->get("k1") << endl;
  cout << db->get("k2") << endl;
}

void batch() {
  shared_ptr<minidb> db(minidb_Factor().get("/tmp")); // 建议使用智能指针
  // 原生指针 记得 delete db; 这个bug调了半天 还以为虚析构函数出问题了
  // minidb *db = minidb_Factor().get("/tmp");
  cout << db->get("k1") << endl;
  cout << db->get("k2") << endl;
  for (int i = 0; i < 1000000; ++i) {
    db->put("k" + to_string(i), "v" + to_string(i));
  }
  cout << db->get("k3") << endl;
  cout << db->get("k4") << endl;
  db->del("k3");
  db->del("k4");
  cout << db->get("k3") << endl;
  cout << db->get("k4") << endl;
}

int main() {
  race();
}
