#include <cassert>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <pthread.h>
#include <sstream>
#include <string>
#include <valarray>
#include <vector>
#include <thread>

#include "db.h"
#include "db_file.h"
#include "entry.h"
#include "minidb.h"
#include "timer/timer.h"

using namespace std;

std::shared_ptr<minidb> minidb_Factor::get(const string &path) {
  auto ptr = std::make_shared<db>(path);
  ptr->gc();
  return ptr;
}

db::db(string path) : df(dbFile(path)), mf(mergeFile(path)), dirPath(path), tws_(50) {
  pthread_mutex_init(&mu, nullptr);
  // 加载索引
  loadIndexesFromFile();
}

db::~db() {
  tws_.Stop();
}

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
#ifdef MUTEX
  pthread_mutex_lock(&mu);
#else
  CWriteLock writeLock(mu_);
#endif
  if (df.size() == 0) {
    return;
  }

  vector<entry> validEntries;
  int64_t offset = 0;
  std::cout << "开始垃圾回收" << std::endl;
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
  std::cout << "垃圾回收结束" << std::endl;
#ifdef MUTEX
  pthread_mutex_unlock(&mu);
#endif
}

void db::gc() {
  tws_.AppendTimeWheel(24, 60 * 60 * 1000, "HourTimeWheel");
  tws_.AppendTimeWheel(60, 60 * 1000, "MinuteTimeWheel");
  tws_.AppendTimeWheel(60, 1000, "SecondTimeWheel");
  tws_.AppendTimeWheel(1000 / 50, 50, "MillisecondTimeWheel");
  tws_.Start();
  tws_.CreateTimerEvery(1000, [this]() {
    std::cout << "开始定时GC" << std::endl;
    this->merge();
  });
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
