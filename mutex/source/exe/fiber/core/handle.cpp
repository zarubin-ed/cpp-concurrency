#include "handle.hpp"

#include "fiber.hpp"

#include <exe/runtime/submit_task.hpp>
#include <exe/runtime/set_timer.hpp>

#include <cassert>
#include <utility>

namespace exe::fiber {

Fiber* FiberHandle::Release() {
  assert(IsValid());
  return std::exchange(fiber_, nullptr);
}

void FiberHandle::Resume() {
  Release()->Resume();
}

void FiberHandle::Schedule() {
  runtime::SubmitTask(fiber_->runtime_, [fiber = Release()]() {
    fiber->Resume();
  });
}

void FiberHandle::Schedule(std::chrono::microseconds delay) {
  runtime::SetTimer(fiber_->runtime_, delay, [fiber = Release()]() {
    fiber->Resume();
  });
}

}  // namespace exe::fiber
