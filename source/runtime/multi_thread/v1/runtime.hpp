#pragma once

#include <exe/runtime/view.hpp>

#include <exe/runtime/timer/timer.hpp>
#include <exe/runtime/timer/scheduler.hpp>

#include "thread_pool.hpp"
#include "timer_thread.hpp"

namespace exe::runtime::multi_thread::v1 {

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
  TimerThread timer_;
};

}  // namespace exe::runtime::multi_thread::v1
