#include "timer_thread.hpp"

#include <twist/ed/std/thread.hpp>

namespace exe::runtime::multi_thread {

using namespace std::chrono_literals;

static constexpr timer::Duration kInaccuracy = 50us;

TimerThread::TimerThread(task::IScheduler* sheduler)
    : sheduler_(sheduler) {
}

void TimerThread::Set(timer::Duration delay, timer::Handler handler) {
  assert(timer_enable_);
  timers_.Push(clock_.now() + delay + kInaccuracy, std::move(handler));
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
  while (auto deadline = timers_.Top()) {
    auto now = clock_.now();
    if (now < *deadline) {
      twist::ed::std::this_thread::sleep_for(*deadline - now);
    }
    sheduler_->Submit(std::move(*timers_.Pop()));
  }
}

}  // namespace exe::runtime::multi_thread
