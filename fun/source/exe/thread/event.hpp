#pragma once

#include <twist/ed/std/atomic.hpp>
#include <twist/ed/wait/futex.hpp>

namespace exe::thread {

class Event {
 public:
  // One-shot
  void Fire() {
    auto wake_key = twist::ed::futex::PrepareWake(state_);
    state_.store(State::Fired);
    twist::ed::futex::WakeAll(wake_key);
  }

  void Wait() {
    while (state_.load() == State::Waiting) {
      twist::ed::futex::Wait(state_, State::Waiting);
    }
  }

 private:
  struct State {
    enum : uint32_t { Fired = 1, Waiting = 0 };
  };

  twist::ed::std::atomic_uint32_t state_{State::Waiting};
};

}  // namespace exe::thread
