#include "RWMutex.h"
#include <iostream> // std::cout/endl

int i = 0;
RWMutex g_rwMutex;

void func1() {
  int iCnt = 100;
  do {
    CReadLock readLock(g_rwMutex);
    ++i;
    std::cout << "func1: " << std::this_thread::get_id() << " " << i
              << std::endl
              << std::flush;
  } while (--iCnt > 0);
}

void func2() {
  int iCnt = 100;
  do {
    CWriteLock writeLock(g_rwMutex);
    ++i;
    std::cout << "func2: " << std::this_thread::get_id() << " " << i
              << std::endl
              << std::flush;
  } while (--iCnt > 0);
}
int main() {
  std::thread thread1(func1);
  thread1.detach();
  std::thread thread2(func1);
  thread2.detach();

  std::thread thread3(func2);
  thread3.detach();
  std::thread thread4(func2);
  thread4.detach();
  getchar();
  return 0;
}
