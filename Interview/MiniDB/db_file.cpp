#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include "db_file.h"
#include "entry.h"

using namespace std;

const uint16_t entryHeaderSize = sizeof(int) +sizeof(int) + sizeof(uint16_t);
const uint16_t extraHeaderSize = sizeof(uint32_t);
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
  char buf[extraHeaderSize] = {0};
  fs.seekg(off);
  fs.read(buf, extraHeaderSize);

  string hs(buf, extraHeaderSize);
  entry e;
  e.decodeSize(hs); // get totalSize
  printf("%d \n",e.totalSize);

  char *ebuf = new char[e.totalSize];
  fs.read(ebuf, e.totalSize);
  string es(ebuf, e.totalSize);

  e.deserialize(es); // get mark key value
  e.print_entry();
  return e;
}

void dbFile::write(entry &e) {
  string s = e.serialize();
  fs.write(s.data(), s.size());
  offset += e.getSize();
}

int main() {
  dbFile df("/home/eintr/.media");

  entry e("123456789", "abcdefghigklmn", 1);
  df.write(e);
  df.read(0);
  df.fs.close();
}
