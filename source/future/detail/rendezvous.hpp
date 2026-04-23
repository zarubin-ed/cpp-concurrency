#pragma once

#include <twist/ed/std/atomic.hpp>

#include <cstdint>

namespace exe::future {

namespace detail {

// Wait-free
class RendezvousStateMachine {
 public:
  // true means rendezvous
  bool Produce() {
    return state_.fetch_or(States::Procuder) == States::Consumer;
  }

  // true means rendezvous
  bool Consume() {
    return state_.fetch_or(States::Consumer) == States::Procuder;
  }

  struct States {
    enum : uint32_t { Init = 0, Procuder = 1, Consumer = 2, Rendezvous = 3 };
  };

  uint32_t State() {
    return state_.load();
  }

 private:
  twist::ed::std::atomic_uint32_t state_{States::Init};
};

}  // namespace detail

}  // namespace exe::future
