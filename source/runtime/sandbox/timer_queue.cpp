#include "timer_queue.hpp"

#include <exe/runtime/task/task.hpp>

namespace exe::runtime::sandbox {

void TimerQueue::Set(timer::Duration delay, TimerBase* timer) {
  timer->deadline = delay + clock_.Now();
  tasks_.Push(timer);
}

bool TimerQueue::IsEmpty() const {
  return tasks_.IsEmpty();
}

std::optional<Instant> TimerQueue::NextDeadline() const {
  return (IsEmpty() ? std::nullopt : std::optional(tasks_.Top()->deadline));
}

void TimerQueue::AdvanceClockBy(timer::Duration delta) {
  clock_.AdvanceBy(delta);
}

void TimerQueue::AdvanceClockToNextDeadline() {
  auto deadline = NextDeadline();
  if (deadline) {
    clock_.AdvanceTo(*deadline);
  }
}

vvv::IntrusiveList<task::TaskBase> TimerQueue::PopReadyTimers() {
  vvv::IntrusiveList<task::TaskBase> ready;
  while (!tasks_.IsEmpty()) {
    if (tasks_.Top()->deadline > clock_.Now()) {
      break;
    }

    ready.PushBack(tasks_.Pop());
  }
  return ready;
}

}  // namespace exe::runtime::sandbox