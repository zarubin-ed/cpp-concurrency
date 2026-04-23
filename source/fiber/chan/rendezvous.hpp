#pragma once

#include "detail/rendezvous_channel_state.hpp"

#include <memory>

namespace exe::fiber {

template <typename T>
class RendezvousChannel {
  using State = detail::RendezvousChannelState<T>;

 public:
  RendezvousChannel()
      : state_(std::make_shared<State>()) {
  }

  void Send(T value) {
    state_->Send(std::move(value));
  }

  T Receive() {
    return state_->Receive();
  }

 private:
  std::shared_ptr<State> state_;
};

}  // namespace exe::fiber
