#pragma once

#include "timer_queue.hpp"

#include <exe/runtime/timer/scheduler.hpp>
#include <exe/runtime/task/scheduler.hpp>

#include <twist/ed/std/thread.hpp>
#include <twist/ed/std/chrono.hpp>

namespace exe::runtime::multi_thread {

class TimerThread final : public timer::IScheduler {
 private:
  using Clock = twist::ed::std::chrono::steady_clock;
  using TimerBase = timer::TimerBase;

 public:
  explicit TimerThread(task::IScheduler*);

  // timer::IScheduler
  void Set(timer::Duration, TimerBase*) override;

  void Start();

  void Stop();

  bool IsTimerEnabled();

  void EnableTimer();

 private:
  void Run();

  bool timer_enable_ = false;
  task::IScheduler* sheduler_;
  TimerBlockingQueue timers_;
  twist::ed::std::thread timer_processor_;
};

}  // namespace exe::runtime::multi_thread
