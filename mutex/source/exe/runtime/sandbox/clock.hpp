#pragma once

#include <cassert>
#include <chrono>

#include <exe/runtime/timer/duration.hpp>

namespace exe::runtime::sandbox {

// Virtual time

using Instant = std::chrono::microseconds;  // Since epoch

class Clock {
 public:
  void AdvanceBy(timer::Duration delta) {
    now_ += delta;
  }

  void AdvanceTo(Instant future) {
    assert(future >= now_);
    now_ = future;
  }

  Instant Now() const {
    return now_;
  }

 private:
  Instant now_{0};
};

}  // namespace exe::runtime::sandbox
