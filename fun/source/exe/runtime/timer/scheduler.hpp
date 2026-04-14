#pragma once

#include "handler.hpp"
#include "duration.hpp"

namespace exe::runtime::timer {

struct IScheduler {
  virtual void Set(Duration delay, Handler) = 0;

 protected:
  ~IScheduler() = default;
};

}  // namespace exe::runtime::timer
