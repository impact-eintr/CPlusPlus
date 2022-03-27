#include <cassert>
#include <cstdint>
#include <map>
#include <pthread.h>
#include <string>
#include <valarray>
#include <vector>
#include <sstream>

#include "db_file.h"
#include "entry.h"

using namespace std;

class minidb {
public:
  minidb(string path);
  ~minidb() {merge();}

  void loadIndexesFromFile();
  void merge();
  void put(string, string);
  string get(string);
  void del(string);

private:
  map<string, int64_t> indexes;
  dbFile df;
  mergeFile mf;
  string dirPath;
  pthread_mutex_t mu;
};

minidb::minidb(string path)
    : df(dbFile(path)), mf(mergeFile(path)), dirPath(path) {
  pthread_mutex_init(&mu, nullptr);
  // 加载索引
  loadIndexesFromFile();
}

void minidb::loadIndexesFromFile() {
  if (df.size() == 0) {
    return;
  }

  int64_t offset = 0;
  while (1) {
    entry e = df.read(offset);
    if (e.getSize() == 0) {
      break;
    }
    // 设置索引状态
    indexes[e.key] = offset;
    if (e.mark == DEL) {
      // 删除内存中的key
      indexes.erase(e.key);
    }
    offset += e.getSize() + extraHeaderSize; // TODO 优化一下封装
  }
}

void minidb::merge() {
  if (df.size() == 0) {
    return;
  }

  vector<entry> validEntries;
  int64_t offset = 0;

  while (1) {
    entry e = df.read(offset);
    if (e.getSize() == 0) {
      break;
    }
    if (indexes.find(e.key) != indexes.end() && offset == indexes[e.key]) {
      validEntries.push_back(e);
    }
    offset += e.getSize() + extraHeaderSize;
  }

  if (validEntries.size() > 0) {
    for (auto v : validEntries) {
      int64_t woff = mf.size();
      mf.mwrite(v);
      indexes[v.key] = woff;
    }
  }
  df._close(dirPath);
  mf._rename(dirPath);
  df._open(dirPath);
}

void minidb::put(string k, string v) {
  if (k.size() == 0) {
    return;
  }

  pthread_mutex_lock(&mu);
  int64_t offset = df.size();
  entry e(k, v, PUT);
  df.write(e);
  indexes[k] = offset;
  pthread_mutex_unlock(&mu);
}

string minidb::get(string k) {
  if (k.size() == 0) {
    return "";
  }

  pthread_mutex_lock(&mu);
  if (indexes.find(k) == indexes.end()) {
    pthread_mutex_unlock(&mu);
    return "";
  }

  int64_t offset = indexes[k];
  entry e = df.read(offset);
  pthread_mutex_unlock(&mu);

  if (e.getSize() != 0) {
    return e.value;
  }
  return "";
}

void minidb::del(string k) {
  if (k.size() == 0) {
    return;
  }

  pthread_mutex_lock(&mu);
  entry e(k, "", DEL);
  df.write(e);
  indexes.erase(k);
  pthread_mutex_unlock(&mu);
}

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
