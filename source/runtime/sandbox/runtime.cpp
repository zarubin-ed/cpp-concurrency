#include "runtime.hpp"

namespace exe::runtime::sandbox {

size_t Runtime::MoveReadyToQueue() {
  size_t res = 0;
  auto ready = timers_.PopReadyTimers();
  while (!ready.IsEmpty()) {
    ++res;
    tasks_.Submit(ready.PopFrontNonEmpty());
  }
  return res;
}

// Run

// Tasks

size_t Runtime::RunAtMostTasks(size_t limit) {
  return tasks_.RunAtMostTasks(limit);
}

size_t Runtime::RunTasks() {
  return tasks_.RunTasks();
}

// Timers

size_t Runtime::AdvanceClockBy(timer::Duration delta) {
  timers_.AdvanceClockBy(delta);
  return MoveReadyToQueue();
}

size_t Runtime::AdvanceClockToNextDeadline() {
  timers_.AdvanceClockToNextDeadline();
  return MoveReadyToQueue();
}

// Empty

bool Runtime::IsEmpty() const {
  return tasks_.IsEmpty() && timers_.IsEmpty();
}

}  // namespace exe::runtime::sandbox
