#include "timer_thread.hpp"

#include <twist/ed/std/thread.hpp>

namespace exe::runtime::multi_thread::v1 {

using namespace std::chrono_literals;

// static constexpr timer::Duration kInaccuracy = 50us;

TimerThread::TimerThread(task::IScheduler* sheduler)
    : sheduler_(sheduler) {
}

void TimerThread::Set(timer::Duration delay, TimerBase* timer) {
  assert(timer_enable_);
  timer->deadline = std::chrono::duration_cast<timer::Instant>(
                        Clock::now().time_since_epoch()) +
                    delay;
  timers_.Push(timer);
}

void TimerThread::Start() {
  timer_processor_ = twist::ed::std::thread([this]() {
    Run();
  });
}

void TimerThread::Stop() {
  timers_.Close();
  timer_processor_.join();
}

bool TimerThread::IsTimerEnabled() {
  return timer_enable_;
}

void TimerThread::EnableTimer() {
  timer_enable_ = true;
}

void TimerThread::Run() {
  while (auto deadline = timers_.NearestDeadLine()) {
    auto now = Clock::now();
    if (now < *deadline) {
      twist::ed::std::this_thread::sleep_for(*deadline - now);
    }
    sheduler_->Submit(timers_.GetClosestTimer());
  }
}

}  // namespace exe::runtime::multi_thread::v1
