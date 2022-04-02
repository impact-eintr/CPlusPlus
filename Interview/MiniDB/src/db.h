#ifndef DB_H_
#define DB_H_

#include "minidb.h"
#include "db_file.h"
#include "entry.h"
#include "RWMutex.h"
#include "timer/time_wheel_scheduler.h"

class db : public minidb {
public:
  db(string path);
  ~db();

  void put(string, string);
  string get(string);
  void del(string);
  void gc();
  void merge();

private:
  void loadIndexesFromFile();

private:
  map<string, int64_t> indexes;
  dbFile df;
  mergeFile mf;
  string dirPath;
  pthread_mutex_t mu;
  RWMutex mu_;
  TimeWheelScheduler tws_;
};

#endif // DB_H_
