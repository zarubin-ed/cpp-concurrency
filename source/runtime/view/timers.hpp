#pragma once

#include <exe/runtime/view.hpp>
#include <exe/runtime/timer/scheduler.hpp>

namespace exe::runtime {

timer::IScheduler& Timers(View);

}  // namespace exe::runtime
