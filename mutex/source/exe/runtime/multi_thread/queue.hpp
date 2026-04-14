#pragma once

#include <exe/thread/primitives.hpp>

#include <twist/ed/std/mutex.hpp>
#include <twist/ed/std/condition_variable.hpp>

#include <algorithm>
#include <optional>
#include <deque>

// Unbounded blocking multi-producers/multi-consumers (MPMC) queue
namespace exe::runtime::multi_thread {

template <typename T>
class UnboundedBlockingQueue {
 public:
  void Push(T value) {
    std::lock_guard guard(mutex_);
    buffer_.emplace_back(std::move(value));
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

 private:
  T LockedPop() {
    T ret = std::move(buffer_.front());
    buffer_.pop_front();
    return ret;
  }

  size_t waiting_ = 0;
  bool is_closed_ = false;
  Mutex mutex_;
  CondVar empty_buffer_;
  std::deque<T> buffer_;
};

}  // namespace exe::runtime::multi_thread