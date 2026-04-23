#pragma once

#include <exe/thread/primitives.hpp>

#include <exe/runtime/detail/intrusive_leftist_heap.hpp>

#include <exe/runtime/timer/timer.hpp>

#include <twist/ed/std/mutex.hpp>
#include <twist/ed/std/condition_variable.hpp>
#include <twist/ed/std/chrono.hpp>

#include <algorithm>
#include <optional>

namespace exe::runtime::multi_thread {

class TimerBlockingQueue {
  using Clock = twist::ed::std::chrono::steady_clock;
  using TimerBase = timer::TimerBase;

 public:
  void Push(TimerBase* timer) {
    std::lock_guard guard(mutex_);

    buffer_.Push(timer);
    if (waiting_ > 0) {
      empty_buffer_.notify_one();
    }
  }

  TimerBase* GetClosestTimer() {
    std::unique_lock lock(mutex_);

    ++waiting_;
    while (!is_closed_ && buffer_.IsEmpty()) {
      empty_buffer_.wait(lock);
    }
    --waiting_;

    if (is_closed_ && buffer_.IsEmpty()) {
      return nullptr;
    }
    return buffer_.Pop();
  }

  void Close() {
    std::lock_guard guard(mutex_);

    is_closed_ = true;
    if (waiting_ > 0) {
      empty_buffer_.notify_all();
    }
  }

  std::optional<Clock::time_point> NearestDeadLine() {
    std::unique_lock lock(mutex_);

    ++waiting_;
    while (!is_closed_ && buffer_.IsEmpty()) {
      empty_buffer_.wait(lock);
    }
    --waiting_;

    if (is_closed_ && buffer_.IsEmpty()) {
      return std::nullopt;
    }

    return Clock::time_point(buffer_.Top()->deadline);
  }

 private:
  size_t waiting_ = 0;
  bool is_closed_ = false;
  Mutex mutex_;
  CondVar empty_buffer_;

  detail::IntrusiveTimerLeftistHeap buffer_;
};

}  // namespace exe::runtime::multi_thread