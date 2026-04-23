#pragma once

#include <exe/runtime/timer/boxed.hpp>
#include <exe/runtime/view/timers.hpp>

namespace exe::runtime {

template <typename F>
void SetTimer(View rt, timer::Duration delay, F handler) {
  Timers(rt).Set(delay, new timer::Boxed<F>(std::move(handler)));
}

}  // namespace exe::runtime
