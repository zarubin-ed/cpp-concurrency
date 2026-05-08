#pragma once

#include "clock.hpp"

#include <exe/detail/intrusive_leftist_heap.hpp>

#include <exe/runtime/timer/timer.hpp>
#include <exe/runtime/timer/scheduler.hpp>

#include <optional>

namespace exe::runtime::sandbox {

class TimerQueue final : public timer::IScheduler {
  using TimerBase = timer::TimerBase;

 public:
  void Set(timer::Duration, TimerBase*) override;

  bool IsEmpty() const;

  std::optional<Instant> NextDeadline() const;

  vvv::IntrusiveList<task::TaskBase> PopReadyTimers();

  void AdvanceClockBy(timer::Duration delta);

  void AdvanceClockToNextDeadline();

 private:
  Clock clock_;
  detail::IntrusiveTimerLeftistHeap tasks_;
};

}  // namespace exe::runtime::sandbox
