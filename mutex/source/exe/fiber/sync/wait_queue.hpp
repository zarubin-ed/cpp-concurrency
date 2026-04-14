#pragma once

#include <exe/fiber/core/handle.hpp>
#include <exe/fiber/sched/suspend.hpp>

#include <queue>

namespace exe::fiber {

class Fiber;

class WaitQueue {
 public:
  template <typename Mutex>
  void Park(Mutex& mutex) {
    Suspend([this, &mutex](FiberHandle handle) {
      fibers_.push(handle);
      mutex.unlock();
    });
    mutex.lock();
  }

  void WakeOne();

  void WakeAll();

  void Push(FiberHandle);

  bool IsEmpty();

 private:
  std::queue<FiberHandle> fibers_;
};

}  // namespace exe::fiber
