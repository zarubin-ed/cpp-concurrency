#pragma once

#include "task.hpp"
#include "hint.hpp"

namespace exe::runtime::task {

struct IScheduler {
  virtual void Submit(TaskBase*, task::SchedulingHint) = 0;

 protected:
  ~IScheduler() = default;
};

}  // namespace exe::runtime::task
