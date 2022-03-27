#ifndef MINIDB_H_
#define MINIDB_H_
#include <string>

class minidb {
public:
  virtual ~minidb() {}

  virtual void merge() = 0;
  virtual void put(std::string, std::string) = 0;
  virtual std::string get(std::string) = 0;
  virtual void del(std::string) = 0;
};

class minidb_Factor {
public:
  minidb *get(const std::string& path);
};

#endif // MINIDB_H_
