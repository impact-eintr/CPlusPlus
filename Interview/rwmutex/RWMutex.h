#ifndef RWMUTEX_H_
#define RWMUTEX_H_

#include <atomic>
#include <cassert>
#include <stdexcept>
#include <thread>

class RWMutex {
  enum { WRITE_LOCK_STATUS = -1, FREE_STATUS = 0 };

public:
  RWMutex(bool writeFirst = false);
  ~RWMutex() = default;
  RWMutex(const RWMutex &) = delete;
  RWMutex &operator=(const RWMutex &) = delete;

  int RLock();
  int RUnlock();
  int Lock();
  int Unlock();

private:
  static const std::thread::id NULL_THEAD; // no.0 thread
  const bool WRITE_FIRST;                   // write first flag
  std::thread::id
  m_writeThreadId; // To determine whether the current writing thread
  std::atomic_int m_lockCount{FREE_STATUS}; // -1 is write state, 0 is free
  // state, > 0 is shared read state
  std::atomic_uint m_writeWaitCount{0};     // waiting to write thread counter
  std::atomic_uint m_readWaitCount{0};      // waiting to read thread counter
};

const std::thread::id RWMutex::NULL_THEAD;

RWMutex::RWMutex(bool writeFirst):WRITE_FIRST(writeFirst), m_writeThreadId() {}


int RWMutex::RLock() {
  if (std::this_thread::get_id() == m_writeThreadId) { // write thread itself want to get rlock
    return m_lockCount;
  }

  // other thread which want to read
  ++m_readWaitCount;
  int count = 0;
  if (WRITE_FIRST) {
    // write first
    do {
      while ((count = m_lockCount.load(std::memory_order_acquire)) == WRITE_LOCK_STATUS || m_writeWaitCount > 0) {
        // Transfer the CPU "time slice A" grabbed by the current thread to other threads,
        // and the "current thread" does not participate in the competition
        std::this_thread::yield();
      }
    } while(!m_lockCount.compare_exchange_weak(count,count + 1,std::memory_order_release));
  } else {
    do {
      while ((count = m_lockCount.load(std::memory_order_acquire)) == WRITE_LOCK_STATUS) {
        std::this_thread::yield();
      }
    } while(!m_lockCount.compare_exchange_weak(count, count + 1, std::memory_order_release));
  }
  --m_readWaitCount;

  return m_lockCount.load(std::memory_order_acquire);
}

int RWMutex::RUnlock() {
  if (std::this_thread::get_id() != m_writeThreadId) {
    return --m_lockCount;
  }
  return m_lockCount.load(std::memory_order_acquire);
}

int RWMutex::Unlock() {
  if (std::this_thread::get_id() !=
      m_writeThreadId) { // read thread want to realese wlock
    throw std::runtime_error("writeLock/Unlock mismatch");
  }
  assert(WRITE_LOCK_STATUS == m_lockCount);

  m_writeThreadId = NULL_THEAD;
  m_lockCount.store(FREE_STATUS, std::memory_order_release);

  return m_lockCount.load(std::memory_order_acquire);
}

int RWMutex::Lock() {
  if (std::this_thread::get_id() ==
      m_writeThreadId) { // write thread itself want to get wlock
    return m_lockCount;
  }

  // other thread which want to write/read
  ++m_writeWaitCount;
  for (int iZero = FREE_STATUS;
     !m_lockCount.compare_exchange_weak(iZero, WRITE_LOCK_STATUS, std::memory_order_release);
       iZero = FREE_STATUS) {
    // On read/write priority, resources must be given up
    std::this_thread::yield();

    while (!WRITE_FIRST  && m_readWaitCount > 0) {
      std::this_thread::yield();
    }
  }
  --m_writeWaitCount;
  m_writeThreadId = std::this_thread::get_id();

  return m_lockCount.load(std::memory_order_acquire);
}

// RAII
template <typename ReadMutex> class ReadLockRAII {
public:
  explicit ReadLockRAII(ReadMutex &lockable) : m_ReadLockable(lockable) {
    m_ReadLockable.RLock();
  }

  ~ReadLockRAII() { m_ReadLockable.RUnlock(); }

private:
  ReadMutex &m_ReadLockable;
};

template <typename WriteMutex> class WriteLockRAII {
public:
  explicit WriteLockRAII(WriteMutex &lockable) : m_WriteLockable(lockable) {
    m_WriteLockable.Lock();
  }

  ~WriteLockRAII() { m_WriteLockable.Unlock(); }

private:
  WriteMutex &m_WriteLockable;
};

// ReadLock
typedef ReadLockRAII<RWMutex> CReadLock;

// WriteLock
typedef WriteLockRAII<RWMutex> CWriteLock;

#endif // RWMUTEX_H
