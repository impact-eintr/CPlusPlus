#ifndef DB_FILE_H_
#define DB_FILE_H_

#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>

#include "entry.h"

extern const string FileName;
extern const string MergeFileName;

class dbFile
{
public:
  dbFile() = default;
  dbFile(string path);
  void mergedbFile(string path);

  // 读写函数
  entry read(uint64_t);
  void write(entry&);

public:
  fstream fs;
  int64_t offset;
};

#endif // DB_FILE_H_
