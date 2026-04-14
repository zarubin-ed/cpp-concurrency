#pragma once

#include <exe/thread/primitives.hpp>

#include <twist/ed/std/mutex.hpp>
#include <twist/ed/std/condition_variable.hpp>
#include <twist/ed/std/chrono.hpp>

#include <algorithm>
#include <optional>
#include <queue>

namespace exe::runtime::multi_thread {

template <typename T>
class TimerBlockingQueue {
  using Clock = twist::ed::std::chrono::steady_clock;

 public:
  void Push(Clock::time_point deadline, T value) {
    std::lock_guard guard(mutex_);
    buffer_.emplace(TimerEntry(deadline, std::move(value)));
    if (waiting_ > 0) {
      empty_buffer_.notify_one();
    }
  }

  std::optional<T> Pop() {
    std::unique_lock lock(mutex_);
    ++waiting_;
    while (buffer_.empty() && !is_closed_) {
      empty_buffer_.wait(lock);
    }
    --waiting_;
    if (is_closed_ && buffer_.empty()) {
      return std::nullopt;
    }
    return LockedPop();
  }

  void Close() {
    std::lock_guard guard(mutex_);
    is_closed_ = true;
    if (waiting_ > 0) {
      empty_buffer_.notify_all();
    }
  }

  std::optional<Clock::time_point> Top() {
    std::unique_lock lock(mutex_);
    ++waiting_;
    while (buffer_.empty() && !is_closed_) {
      empty_buffer_.wait(lock);
    }
    --waiting_;
    if (is_closed_ && buffer_.empty()) {
      return std::nullopt;
    }
    return LockedTop();
  }

 private:
  struct TimerEntry {
    Clock::time_point deadline;
    mutable T value;

    // TimerEntry(Clock::time_point d, timer::Handler h)
    // : deadline(d), handler(std::move(h)) {}

    bool operator<(const TimerEntry& other) const {
      return deadline > other.deadline;
    }
  };

  Clock::time_point LockedTop() {
    return buffer_.top().deadline;
  }

  T LockedPop() {
    T ret = std::move(buffer_.top().value);
    buffer_.pop();
    return ret;
  }

  size_t waiting_ = 0;
  bool is_closed_ = false;
  Mutex mutex_;
  CondVar empty_buffer_;
  std::priority_queue<TimerEntry> buffer_;
};

}  // namespace exe::runtime::multi_thread