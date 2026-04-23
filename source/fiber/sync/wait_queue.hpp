#pragma once
#include "handle_node.hpp"

#include <exe/fiber/sched/suspend.hpp>

namespace exe::fiber {

class WaitQueue {
 public:
  template <typename Mutex>
  void Park(Mutex& mutex) {
    std::optional<FiberHandleNode> node;

    Suspend([this, &mutex, &node](FiberHandle handle) {
      node.emplace(handle);

      fibers_.PushBack(&*node);

      mutex.unlock();
    });

    mutex.lock();
  }

  void WakeOne();

  void WakeAll();

  bool IsEmpty();

 private:
  vvv::IntrusiveList<FiberHandleNode> fibers_;
};

}  // namespace exe::fiber
