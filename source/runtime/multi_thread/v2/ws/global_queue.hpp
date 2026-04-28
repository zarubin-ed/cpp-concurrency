#pragma once

#include <exe/runtime/task/task.hpp>

#include <exe/thread/spinlock.hpp>

#include <twist/ed/std/mutex.hpp>

#include <span>

namespace exe::runtime::multi_thread::v2 {

// Unbounded task queue shared between workers

class GlobalTaskQueue {
 public:
  void PushOne(task::TaskBase* task) {
    twist::ed::std::lock_guard guard(spin_);
    tasks_.PushBack(task);
  }

  void PushMany(std::span<task::TaskBase*> buffer) {
    twist::ed::std::lock_guard guard(spin_);

    for (auto task : buffer) {
      tasks_.PushBack(task);
    }
  }

  task::TaskBase* TryPopOne() {
    twist::ed::std::lock_guard guard(spin_);
    return tasks_.TryPopFront();
  }

  size_t Grab(std::span<task::TaskBase*> out_buffer) {
    twist::ed::std::lock_guard guard(spin_);
    size_t res = 0;
    for (auto& item : out_buffer) {
      item = tasks_.TryPopFront();
      if (item == nullptr) {
        break;
      }
      ++res;
    }
    return res;
  }

 private:
  vvv::IntrusiveList<task::TaskBase> tasks_;

  thread::SpinLock spin_;
};

}  // namespace exe::runtime::multi_thread::v2
