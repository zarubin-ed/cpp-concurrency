#pragma once

#include <twist/ed/wait/spin.hpp>
#include <twist/ed/std/atomic.hpp>

namespace exe::thread {

class SpinLock {
 public:
  void Lock() {
    twist::ed::SpinWait spin_wait;
    while (locked_.exchange(1) == 1) {
      while (locked_.load() == 1) {
        spin_wait();
      }
    }
  }

  bool TryLock() {
    return locked_.exchange(1) == 0;
  }

  void Unlock() {
    locked_.store(0);
  }

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
  twist::ed::std::atomic_uint32_t locked_{0};
};

}  // namespace exe::thread