#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include "db_file.h"
#include "entry.h"

using namespace std;

const uint64_t entryHeaderSize = 10;
const string FileName = "minidb.data";
const string MergeFileName = "minidb.merge";

dbFile::dbFile(string path) {
  string fileName = path + "/" + FileName;
  fs.open(fileName, ios::in | ios::app);
  fs.seekg(0, fs.end);
  offset = fs.tellg();
  fs.seekg(0, fs.beg);
}

void dbFile::mergedbFile(string path) {
  string fileName = path + "/" + MergeFileName;
  fs.open(fileName, ios::in | ios::app);
  fs.seekg(0, fs.end);
  offset = fs.tellg();
  fs.seekg(0, fs.beg);
}

entry dbFile::read(uint64_t off) {
  char buf[entryHeaderSize] = {0};
  fs.seekg(off);
  fs.read(buf, entryHeaderSize);

  string s;
  for (int i = 0;i < entryHeaderSize;++i)
    s.push_back(buf[i]);
  entry e;
  e.deserialize(s); // get ks vs mark
  char *kbuf = new char[e.keySize];
  char *vbuf = new char[e.valueSize];
  printf("%d %d\n",e.keySize, e.valueSize);

  fs.read(buf, sizeof(int)); // 跳过内部标记
  fs.read(kbuf, e.keySize);
  fs.read(buf, sizeof(int));// 跳过内部标记
  fs.read(vbuf, e.valueSize);

  e.key = kbuf;
  e.value = vbuf;
  e.print_entry();
  return e;
}

void dbFile::write(entry &e) {
  string s = e.serialize();
  fs.write(s.data(), s.size());
  offset += e.getSize();
}

//int main() {
//  dbFile df("/home/eintr/.media");
//
//  entry e("123456789", "abcdefghigklmn", 1);
//  df.write(e);
//  df.read(0);
//  df.fs.close();
//}
