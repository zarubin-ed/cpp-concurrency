#pragma once

#include <exe/thread/primitives.hpp>

#include <twist/ed/std/mutex.hpp>
#include <twist/ed/std/condition_variable.hpp>

#include <vvv/list.hpp>

#include <algorithm>
#include <optional>

// Unbounded blocking multi-producers/multi-consumers (MPMC) queue
namespace exe::runtime::multi_thread {

template <typename T>
class UnboundedBlockingQueue {
 public:
  void Push(T* value) {
    std::lock_guard guard(mutex_);

    buffer_.PushBack(value);

    if (waiting_ > 0) {
      empty_buffer_.notify_one();
    }
  }

  std::optional<T*> Pop() {
    std::unique_lock lock(mutex_);

    ++waiting_;
    while (!is_closed_ && buffer_.IsEmpty()) {
      empty_buffer_.wait(lock);
    }
    --waiting_;

    if (is_closed_ && buffer_.IsEmpty()) {
      return std::nullopt;
    }

    return buffer_.PopFrontNonEmpty();
  }

  void Close() {
    std::lock_guard guard(mutex_);

    is_closed_ = true;
    if (waiting_ > 0) {
      empty_buffer_.notify_all();
    }
  }

 private:
  vvv::IntrusiveList<T> buffer_;

  Mutex mutex_;
  CondVar empty_buffer_;

  size_t waiting_ = 0;
  bool is_closed_ = false;
};

}  // namespace exe::runtime::multi_thread