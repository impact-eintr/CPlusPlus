#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <cassert>
#include <sys/stat.h>

#include "db_file.h"
#include "entry.h"

using namespace std;

const uint16_t entryHeaderSize = sizeof(int) +sizeof(int) + sizeof(uint16_t);
const uint16_t extraHeaderSize = sizeof(uint32_t);
const string FileName = "minidb.data";
const string MergeFileName = "minidb.merge";

dbFile::dbFile(const string& path) {
  string fileName = path + "/" + FileName;
  fs.open(fileName, ios::in | ios::app);
  struct stat res;
  if (stat(fileName.data(), &res) == 0)
    offset = res.st_size;
}

entry dbFile::read(int64_t off) {
  entry e;
  if (offset <= off) {
    return e;
  }

  // 读取extraHeader 获取需要读的字节数
  char buf[extraHeaderSize] = {0};
  fs.seekg(off);
  fs.read(buf, extraHeaderSize);
  string hs(buf, extraHeaderSize);
  e.decodeSize(hs); // get getSize()

  // 读取相应长度的数据 并反序列化
  char *ebuf = new char[e.getSize()];
  fs.read(ebuf, e.getSize());
  string es(ebuf, e.getSize());
  delete [] ebuf;
  e.deserialize(es); // get mark key value

  return e;
}

void dbFile::write(entry &e) {
  string s = e.serialize();
  fs << s;
  offset += e.getSize() + extraHeaderSize;
}

mergeFile::mergeFile(const string& path) {
  string fileName = path + "/" + MergeFileName;
  fs.open(fileName, ios::in | ios::out | ios::trunc);
  struct stat res;
  if (stat(fileName.data(), &res) == 0)
    offset = res.st_size;
}

entry mergeFile::mread(int64_t off) {
  return read(off);
}

void mergeFile::mwrite(entry &e) {
  write(e);
}
