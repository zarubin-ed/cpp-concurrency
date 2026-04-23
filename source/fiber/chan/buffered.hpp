#pragma once

#include "detail/buffered_channel_state.hpp"

#include <memory>

namespace exe::fiber {

template <typename T>
class BufferedChannel {
  using State = detail::BufferedChannelState<T>;

 public:
  explicit BufferedChannel(size_t capacity)
      : state_(std::make_shared<State>(capacity)) {
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
