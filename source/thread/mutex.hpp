#pragma once

#include <twist/ed/std/atomic.hpp>
#include <twist/ed/wait/futex.hpp>
#include <twist/ed/wait/spin.hpp>

#include <cstdint>

namespace primitives {

class Mutex {
 public:
  void Lock() {
    uint32_t expected = 0;

    if (is_vacant_.compare_exchange_strong(expected, Locked)) {
      return;
    }

    while (is_vacant_.exchange(Waiting) != Unlocked) {
      twist::ed::futex::Wait(is_vacant_, Waiting);
    }
  }

  void Unlock() {
    auto wake_key = twist::ed::futex::PrepareWake(is_vacant_);
    if (is_vacant_.exchange(Unlocked) == Waiting) {
      twist::ed::futex::WakeOne(wake_key);
    }
  }

  // BasicLockable
  // https://en.cppreference.com/w/cpp/named_req/BasicLockable

  void lock() {  // NOLINT
    Lock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  enum MutexStates : uint32_t { Unlocked = 0, Locked = 1, Waiting = 2 };

  twist::ed::std::atomic<uint32_t> is_vacant_{0};
};

};  // namespace primitives