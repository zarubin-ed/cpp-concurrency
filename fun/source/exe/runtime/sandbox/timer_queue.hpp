#pragma once

#include "clock.hpp"

#include <exe/runtime/timer/handler.hpp>
#include <exe/runtime/timer/scheduler.hpp>

#include <optional>
#include <queue>

namespace exe::runtime::sandbox {

class TimerQueue final : public timer::IScheduler {
 public:
  void Set(timer::Duration delay, timer::Handler);

  void Push(Instant, timer::Handler);

  bool IsEmpty() const;

  std::optional<Instant> NextDeadline() const;

  std::vector<timer::Handler> PopReadyTimers();

  void AdvanceClockBy(timer::Duration delta);

  void AdvanceClockToNextDeadline();

 private:
  struct TimerEntry {
    Instant deadline;
    mutable timer::Handler handler;

    bool operator<(const TimerEntry& other) const {
      return deadline > other.deadline;
    }
  };

  Clock clock_;
  std::priority_queue<TimerEntry> tasks_;
};

}  // namespace exe::runtime::sandbox
