#include "sleep_for.hpp"
#include "suspend.hpp"

#include <exe/runtime/timer/timer.hpp>

namespace exe::fiber {

void SleepFor(std::chrono::microseconds delay) {
  runtime::timer::TimerBase timer;

  auto callback = [timer = &timer, delay](FiberHandle handle) {
    handle.Schedule(delay, timer);
  };

  Suspend(Callback(callback));
}

}  // namespace exe::fiber
