#include "runtime.hpp"

namespace exe::runtime::sandbox {

size_t Runtime::MoveReadyToQueue() {
  auto ready = timers_.PopReadyTimers();
  for (auto& task : ready) {
    tasks_.Submit(std::move(task));
  }
  return ready.size();
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
