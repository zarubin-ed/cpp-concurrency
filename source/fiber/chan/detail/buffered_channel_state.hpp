#pragma once

#include "circular_buffer.hpp"

#include <exe/fiber/sync/wait_queue.hpp>
#include <exe/thread/spinlock.hpp>
#include <exe/thread/unique_lock.hpp>

#include <cassert>

namespace exe::fiber {

namespace detail {

template <typename T>
class BufferedChannelState {
 public:
  explicit BufferedChannelState(size_t capacity)
      : capacity_(capacity),
        buffer_(capacity_) {
    assert(capacity > 0);
  }

  ~BufferedChannelState() {
    assert(senders_.IsEmpty());
    assert(receivers_.IsEmpty());
  }

  void Send(T val) {
    thread::UniqueLock lock(spin_);

    TryRendezvous(val, lock);

    while (buffer_.IsFull()) {
      senders_.Park(lock);
    }

    receivers_.WakeOne();

    buffer_.Emplace(std::move(val));
  }

  T Receive() {
    thread::UniqueLock lock(spin_);

    while (buffer_.IsEmpty() && fast_pass_message_ == nullptr) {
      receivers_.Park(lock);
    }

    if (fast_pass_message_ != nullptr) {  // Rendezvous
      partner_.WakeOne();
      return std::move(*fast_pass_message_);
    }

    senders_.WakeOne();

    return buffer_.Pop();
  }

 private:
  void TryRendezvous(T& val, thread::UniqueLock<thread::SpinLock>& lock) {
    while (!receivers_.IsEmpty() &&
           fast_pass_message_ != nullptr) {  // Rendezvous
      senders_.Park(lock);
    }

    if (!receivers_.IsEmpty()) {
      fast_pass_message_ = &val;
      receivers_.WakeOne();

      partner_.Park(lock);
      fast_pass_message_ = nullptr;

      senders_.WakeOne();
      return;
    }
  }

  size_t capacity_;

  WaitQueue senders_;
  WaitQueue partner_;
  WaitQueue receivers_;

  thread::SpinLock spin_;

  CircularBuffer<T> buffer_;
  T* fast_pass_message_ = nullptr;
};

}  // namespace detail

}  // namespace exe::fiber
