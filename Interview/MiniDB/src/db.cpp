#include <cassert>
#include <cstdint>
#include <map>
#include <pthread.h>
#include <sstream>
#include <string>
#include <valarray>
#include <vector>
#include <thread>

#include "db.h"
#include "db_file.h"
#include "entry.h"

using namespace std;

minidb *minidb_Factor::get(const string &path) { return new db(path); }

db::db(string path) : df(dbFile(path)), mf(mergeFile(path)), dirPath(path) {
  pthread_mutex_init(&mu, nullptr);
  // 加载索引
  loadIndexesFromFile();
}

db::~db() { merge(); }

void db::loadIndexesFromFile() {
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
    offset += e.getSize() + extraHeaderSize;
  }
}

void db::merge() {
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

void db::put(string k, string v) {
  if (k.size() == 0) {
    return;
  }

#ifdef MUTEX
  pthread_mutex_lock(&mu);
#else
  CWriteLock writeLock(mu_);
#endif
  int64_t offset = df.size();
  entry e(k, v, PUT);
  df.write(e);
  indexes[k] = offset;
#ifdef MUTEX
  pthread_mutex_unlock(&mu);
#endif
}

string db::get(string k) {
  if (k.size() == 0) {
    return "";
  }
#ifdef MUTEX
  pthread_mutex_lock(&mu);
#else
  CReadLock readLock(mu_);
#endif
  if (indexes.find(k) == indexes.end()) {
#ifdef MUTEX
    pthread_mutex_unlock(&mu);
#endif
    return "";
  }

  int64_t offset = indexes[k];
  entry e = df.read(offset);
#ifdef MUTEX
  pthread_mutex_unlock(&mu);
#endif

  if (e.getSize() != 0) {
    return e.value;
  }
  return "";
}

void db::del(string k) {
  if (k.size() == 0) {
    return;
  }

#ifdef MUTEX
  pthread_mutex_lock(&mu);
#else
  CWriteLock writeLock(mu_);
#endif
  entry e(k, "", DEL);
  df.write(e);
  indexes.erase(k);
#ifdef MUTEX
  pthread_mutex_unlock(&mu);
#endif
}
