#include "runtime.hpp"

#include <exe/runtime/timer/timer.hpp>
#include <exe/runtime/task/task.hpp>

namespace exe::runtime::run_loop {

task::TaskBase* Runtime::GetTask() {
  do {
    auto deadline = timers_.TryNearestDeadLine();
    if (deadline && Clock::now() >= *deadline) {
      return timers_.GetClosestTimer();
    }

    if (auto task = tasks_.Pop()) {
      return *task;
    }
  } while (!is_closed_.load());
  return nullptr;
}

void Runtime::Run() {
  while (auto task = GetTask()) {
    task->Run();
  }
}

void Runtime::Stop() {
  timers_.Close();
  tasks_.Close();
  is_closed_.store(true);
}

////////////////////////////////////////////////////////////
/// timer::IScheduler

using namespace std::chrono_literals;

void Runtime::Set(timer::Duration delay, timer::TimerBase* timer) {
  timer->deadline = std::chrono::duration_cast<timer::Instant>(
                        Clock::now().time_since_epoch()) +
                    delay;
  timers_.Push(timer);
}

////////////////////////////////////////////////////////////
/// task::IScheduler

void Runtime::Submit(task::TaskBase* task, task::SchedulingHint) {
  tasks_.Push(task);
}
}  // namespace exe::runtime::run_loop
