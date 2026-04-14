#pragma once

#include <exe/runtime/view/timers.hpp>

namespace exe::runtime {

inline void SetTimer(View rt, timer::Duration delay, timer::Handler handler) {
  Timers(rt).Set(delay, std::move(handler));
}

}  // namespace exe::runtime
