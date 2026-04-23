#pragma once

#include <exe/runtime/task/task.hpp>

#include <twist/ed/std/chrono.hpp>

namespace exe::runtime::timer {
using Instant = std::chrono::microseconds;

struct TimerBase : task::TaskBase {
  task::TaskBase* task = nullptr;
  Instant deadline;
  size_t depth = 0;

  void Run() noexcept override {
    std::exchange(task, nullptr)->Run();
  }
};

}  // namespace exe::runtime::timer
