#pragma once

#include "timer_queue.hpp"
#include "manual_loop.hpp"

#include <exe/runtime/view.hpp>

#include <exe/runtime/timer/duration.hpp>

namespace exe::runtime::sandbox {

class Runtime {
 public:
  Runtime() = default;

  // NOLINTNEXTLINE
  operator View() {
    return {&tasks_, &timers_};
  }

  // Run

  // Tasks

  size_t RunAtMostTasks(size_t /* limit */);

  bool RunNextTask() {
    return RunAtMostTasks(1) == 1;
  }

  size_t RunTasks();

  // Timers

  size_t AdvanceClockBy(timer::Duration delta);

  size_t AdvanceClockToNextDeadline();

  // Empty

  bool IsEmpty() const;

  bool NonEmpty() const {
    return !IsEmpty();
  }

 private:
  size_t MoveReadyToQueue();

  ManualLoop tasks_;
  TimerQueue timers_;
};

}  // namespace exe::runtime::sandbox
