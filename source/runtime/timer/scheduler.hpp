#pragma once

#include "duration.hpp"
#include "timer.hpp"

namespace exe::runtime::timer {

struct IScheduler {
  virtual void Set(Duration delay, TimerBase*) = 0;

 protected:
  ~IScheduler() = default;
};

}  // namespace exe::runtime::timer
