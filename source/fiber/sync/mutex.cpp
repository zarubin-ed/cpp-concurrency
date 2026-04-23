#include "mutex.hpp"

#include <exe/thread/unique_lock.hpp>

#include <twist/ed/std/mutex.hpp>

#include <utility>

namespace exe::fiber {

void Mutex::Lock() {
  thread::UniqueLock lock(spin_);

  while (std::exchange(is_locked_, true)) {
    fibers_.Park(lock);
  }
}

bool Mutex::TryLock() {
  twist::ed::std::lock_guard guard(spin_);
  return !std::exchange(is_locked_, true);
}

void Mutex::Unlock() {
  twist::ed::std::lock_guard guard(spin_);

  is_locked_ = false;
  fibers_.WakeOne();
}

}  // namespace exe::fiber