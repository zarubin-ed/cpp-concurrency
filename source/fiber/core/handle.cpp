#include "handle.hpp"

#include "fiber.hpp"

#include <exe/runtime/view/tasks.hpp>
#include <exe/runtime/view/timers.hpp>
#include <exe/runtime/timer/timer.hpp>
#include <exe/runtime/timer/duration.hpp>

#include <cassert>
#include <utility>

namespace exe::fiber {

Fiber* FiberHandle::Release() {
  assert(IsValid());
  return std::exchange(fiber_, nullptr);
}

void FiberHandle::Schedule(runtime::task::SchedulingHint hint) {
  auto fiber = Release();
  runtime::Tasks(fiber->runtime_).Submit(fiber, hint);
}

void FiberHandle::Schedule(runtime::timer::Duration delay,
                           runtime::timer::TimerBase* timer) {
  auto fiber = Release();
  timer->task = fiber;

  runtime::Timers(fiber->runtime_).Set(delay, timer);
}

}  // namespace exe::fiber
