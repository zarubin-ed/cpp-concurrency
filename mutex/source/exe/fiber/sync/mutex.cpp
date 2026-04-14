#include "mutex.hpp"

#include <exe/thread/unique_lock.hpp>

#include <twist/ed/std/mutex.hpp>

namespace exe::fiber {

void Mutex::Lock() {
  thread::UniqueLock lock(spin_);

  if (is_locked_) {
    fibers_.Park(lock);
  } else {
    is_locked_ = true;
  }
}

bool Mutex::TryLock() {
  twist::ed::std::lock_guard guard(spin_);

  if (is_locked_) {
    return false;
  }

  return is_locked_ = true;
}

void Mutex::Unlock() {
  twist::ed::std::lock_guard guard(spin_);

  if (fibers_.IsEmpty()) {
    is_locked_ = false;
  } else {
    fibers_.WakeOne();
  }
}

}  // namespace exe::fiber