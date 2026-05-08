#pragma once

#include <exe/detail/timer_queue.hpp>
#include <exe/detail/queue.hpp>

#include <exe/runtime/view.hpp>
#include <exe/runtime/task/scheduler.hpp>
#include <exe/runtime/timer/scheduler.hpp>

#include <twist/ed/std/atomic.hpp>
#include <twist/ed/std/mutex.hpp>
#include <twist/ed/std/condition_variable.hpp>

namespace exe::runtime::run_loop {

class Runtime : public task::IScheduler,
                public timer::IScheduler {
  using Clock = twist::ed::std::chrono::steady_clock;

 public:
  Runtime() = default;

  // NOLINTNEXTLINE
  operator View() {
    return {this, this};
  }

  task::TaskBase* GetTask();

  void Run();
  void Stop();

  void Submit(task::TaskBase*, task::SchedulingHint) override;

  void Set(timer::Duration delay, timer::TimerBase*) override;

 private:
  detail::TimerBlockingQueue timers_;
  detail::UnboundedBlockingQueue<task::TaskBase> tasks_;

  twist::ed::std::atomic<bool> is_closed_{false};
};

}  // namespace exe::runtime::run_loop
