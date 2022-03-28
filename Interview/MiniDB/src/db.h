#ifndef DB_H_
#define DB_H_

#include "minidb.h"
#include "db_file.h"
#include "entry.h"
#include "RWMutex.h"

class db : public minidb {
public:
  db(string path);
  ~db();

  void merge();
  void put(string, string);
  string get(string);
  void del(string);

private:
  void loadIndexesFromFile();

private:
  map<string, int64_t> indexes;
  dbFile df;
  mergeFile mf;
  string dirPath;
  pthread_mutex_t mu;
  RWMutex mu_;
};

#endif // DB_H_
