#pragma once
#include "wait_queue.hpp"

#include <exe/thread/spinlock.hpp>

namespace exe::fiber {

class Mutex {
 public:
  void Lock();

  bool TryLock();

  void Unlock();

  // Lockable

  void lock() {  // NOLINT
    Lock();
  }

  bool try_lock() {  // NOLINT
    return TryLock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  WaitQueue fibers_;
  thread::SpinLock spin_;
  bool is_locked_ = false;
};

}  // namespace exe::fiber
