#ifndef DB_FILE_H_
#define DB_FILE_H_

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#include "entry.h"

extern const string FileName;
extern const string MergeFileName;

class dbFile {
public:
  dbFile() = default;
  dbFile(const string &path);

  inline int64_t size() { return offset; }
  inline void _open(const string &path) {
    string fileName = path + "/" + FileName;
    fs.open(fileName, ios::in | ios::app);
    struct stat res;
    if (stat(fileName.data(), &res) == 0)
      offset = res.st_size;
  }
  inline void _close(const string &path) { fs.close(); }
  entry read(int64_t);
  void write(entry &);

protected:
  int64_t offset;
  fstream fs;
};

class mergeFile : dbFile {
public:
  mergeFile(const string &path);

  inline int64_t size() { return offset; }
  entry mread(int64_t);
  void mwrite(entry &);

  inline void _open(const string&) = delete;
  inline void _close(const string&) = delete;
  inline void _rename(const string &path) {
    fs.close();
    string fileName = path + "/" + FileName;
    string mergefileName = path + "/" + MergeFileName;
    remove(fileName.data());
    rename(mergefileName.data(), fileName.data());
  }
};

#endif // DB_FILE_H_
