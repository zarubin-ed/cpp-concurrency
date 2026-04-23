#include "wait_queue.hpp"
#include <exe/thread/spinlock.hpp>
#include <exe/thread/unique_lock.hpp>

#include <twist/ed/std/mutex.hpp>

#include <cstddef>

namespace exe::fiber {

class WaitGroup {
 public:
  void Add(size_t count) {
    twist::ed::std::lock_guard guard(spin_);
    count_ += count;
  }

  void Done() {
    twist::ed::std::lock_guard guard(spin_);
    --count_;

    if (count_ == 0) {
      fibers_.WakeAll();
    }
  }

  void Wait() {
    thread::UniqueLock lock(spin_);

    if (count_ != 0) {
      fibers_.Park(lock);
    }
  }

 private:
  thread::SpinLock spin_;
  WaitQueue fibers_;
  size_t count_ = 0;
};

}  // namespace exe::fiber
