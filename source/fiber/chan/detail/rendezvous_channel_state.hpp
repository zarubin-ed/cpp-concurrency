#pragma once

#include "state.hpp"
#include "manual_storage.hpp"

#include <exe/fiber/core/handle.hpp>
#include <exe/fiber/sched/suspend.hpp>

#include <exe/runtime/task/hint.hpp>

#include <exe/thread/spinlock.hpp>
#include <exe/thread/unique_lock.hpp>

#include <vvv/list.hpp>

namespace exe::fiber {

namespace detail {

template <typename T>
class RendezvousChannelState {
 public:
  ~RendezvousChannelState() {
    assert(waiters_.IsEmpty());
  }

  void Send(T val) {
    thread::UniqueLock lock(spin_);

    if (state_ == States::Receivers) {
      auto receiver = waiters_.PopFrontNonEmpty();
      std::construct_at(receiver->message, std::move(val));

      receiver->handle.Schedule(runtime::task::SchedulingHint::Next);

      if (waiters_.IsEmpty()) {
        state_ = States::Nobody;
      }
    } else {
      WaiterNode<T> sender;
      sender.message = &val;

      auto callback = [this, &sender, &lock](FiberHandle handle) {
        sender.handle = handle;
        waiters_.PushBack(&sender);
        state_ = States::Senders;

        lock.unlock();
      };
      Suspend(Callback(callback));

      lock.lock();
    }
  }

  T Receive() {
    thread::UniqueLock lock(spin_);
    ManualStorage<T> res;

    if (state_ == States::Senders) {
      auto sender = waiters_.PopFrontNonEmpty();
      res.Emplace(std::move(*sender->message));

      sender->handle.Schedule(runtime::task::SchedulingHint::Next);
      if (waiters_.IsEmpty()) {
        state_ = States::Nobody;
      }
    } else {
      WaiterNode<T> receiver;
      receiver.message = &*res;

      auto callback = [this, &receiver, &lock](FiberHandle handle) {
        receiver.handle = handle;
        waiters_.PushBack(&receiver);
        state_ = States::Receivers;

        lock.unlock();
      };
      Suspend(Callback(callback));

      lock.lock();
    }

    return std::move(*res);
  }

 private:
  vvv::IntrusiveList<WaiterNode<T>> waiters_;

  thread::SpinLock spin_;
  uint32_t state_ = States::Nobody;
};

}  // namespace detail

}  // namespace exe::fiber
