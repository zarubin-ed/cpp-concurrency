#pragma once
#include "manual_storage.hpp"

#include <exe/fiber/sync/wait_queue.hpp>

#include <exe/thread/spinlock.hpp>
#include <exe/thread/unique_lock.hpp>

#include <cstdlib>  // std::abort
#include <queue>

namespace exe::fiber {

namespace detail {

struct RendezvousStateMachine {
  struct State {
    enum : int {
      Init = 0,
      Sent = 1,
      Recieved = 2,
      Rendezvous = Sent | Recieved
    };
  };

  int State() {
    return state_;
  }

  bool Send() {
    return (state_ |= State::Sent) == State::Rendezvous;
  }

  bool Receive() {
    return (state_ |= State::Recieved) == State::Rendezvous;
  }

  bool Sent() {
    return (state_ & State::Sent) != 0;
  }

  bool Received() {
    return (state_ & State::Recieved) != 0;
  }

  void Reset() {
    state_ = State::Init;
  }

  bool IsRendezvous() const {
    return state_ == State::Rendezvous;
  }

 private:
  int state_ = State::Init;
};

template <typename T>
class RendezvousChannelState {
 public:
  ~RendezvousChannelState() {
    assert(senders_.IsEmpty());
    assert(receivers_.IsEmpty());
  }

  void Send(T val) {
    thread::UniqueLock lock(spin_);

    while (state_.Sent()) {
      senders_.Park(lock);
    }

    if (state_.Send()) {
      std::construct_at(message_, std::move(val));
      partner_.WakeOne();
    } else {
      message_ = &val;
      partner_.Park(lock);
    }

    Reset();
  }

  T Receive() {
    thread::UniqueLock lock(spin_);

    while (state_.Received()) {
      receivers_.Park(lock);
    }

    if (state_.Receive()) {
      partner_.WakeOne();
      return std::move(*message_);
    } else {
      ManualStorage<T> ret;
      message_ = &*ret;

      partner_.Park(lock);
      return std::move(*ret);
    }
  }

 private:
  void Reset() {
    state_.Reset();
    message_ = nullptr;
    receivers_.WakeOne();
    senders_.WakeOne();
  }

  WaitQueue senders_;
  WaitQueue partner_;
  WaitQueue receivers_;

  thread::SpinLock spin_;

  T* message_ = nullptr;
  RendezvousStateMachine state_;
};

}  // namespace detail

}  // namespace exe::fiber
