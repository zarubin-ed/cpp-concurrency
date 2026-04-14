#include "timer_queue.hpp"

namespace exe::runtime::sandbox {

void TimerQueue::Set(timer::Duration delay, timer::Handler handler) {
  Push(delay + clock_.Now(), std::move(handler));
}

void TimerQueue::Push(Instant deadline, timer::Handler handler) {
  tasks_.push(TimerEntry{deadline, std::move(handler)});
}

bool TimerQueue::IsEmpty() const {
  return tasks_.empty();
}

std::optional<Instant> TimerQueue::NextDeadline() const {
  return (IsEmpty() ? std::nullopt : std::optional(tasks_.top().deadline));
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

std::vector<timer::Handler> TimerQueue::PopReadyTimers() {
  std::vector<timer::Handler> ready;
  while (!tasks_.empty()) {
    if (tasks_.top().deadline > clock_.Now()) {
      break;
    }

    ready.emplace_back(std::move(tasks_.top().handler));
    tasks_.pop();
  }
  return ready;
}

}  // namespace exe::runtime::sandbox