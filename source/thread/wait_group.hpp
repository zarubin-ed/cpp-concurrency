#pragma once

#include "primitives.hpp"

#include <twist/ed/std/mutex.hpp>  // for lock_guard & unique_lock

#include <cstddef>

namespace exe::thread {

class WaitGroup {
 public:
  void Add(size_t count) {
    twist::ed::std::lock_guard lock(mutex_);
    count_ += count;
  }

  void Done() {
    twist::ed::std::lock_guard lock(mutex_);
    --count_;
    if (waiting_ > 0) {
      zero_counter_.notify_all();
    }
  }

  void Wait() {
    twist::ed::std::unique_lock lock(mutex_);
    ++waiting_;
    while (count_ != 0) {
      zero_counter_.wait(lock);
    }
    --waiting_;
  }

 private:
  size_t waiting_ = 0;
  size_t count_ = 0;
  Mutex mutex_;
  CondVar zero_counter_;
};

}  // namespace exe::thread
