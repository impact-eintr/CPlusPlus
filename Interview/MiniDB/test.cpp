#include "minidb.h"

int main() {
  minidb db("/tmp");
  cout << db.get("k1") << endl;
  cout << db.get("k2") << endl;
  for (int i = 0;i < 1000000;++i) {
    db.put("k"+to_string(i), "v"+to_string(i));
  }
  cout << db.get("k3") << endl;
  cout << db.get("k4") << endl;
  db.del("k3");
  db.del("k4");
  cout << db.get("k3") << endl;
  cout << db.get("k4") << endl;
}
