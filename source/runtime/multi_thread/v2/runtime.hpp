#pragma once

#include <exe/runtime/view.hpp>
#include <exe/runtime/multi_thread/v1/timer_thread.hpp>

#include "ws/thread_pool.hpp"

namespace exe::runtime::multi_thread::v2 {

class Runtime {
 public:
  explicit Runtime(size_t /*num_workers*/);

  // NOLINTNEXTLINE
  operator View() {
    return {&tasks_, &timer_};
  }

  Runtime& WithTimers();

  void Start();

  void Stop();

  bool Here() const;

 private:
  ThreadPool tasks_;
  v1::TimerThread timer_;
};

}  // namespace exe::runtime::multi_thread::v2
