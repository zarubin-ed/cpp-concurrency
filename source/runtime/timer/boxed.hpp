#pragma once

#include "timer.hpp"

#include <functional>
#include <utility>

namespace exe::runtime::timer {

template <typename F>
class Boxed final : public TimerBase {
 public:
  explicit Boxed(F task)
      : task_(std::move(task)) {
  }

  void Run() noexcept override {
    std::invoke(std::move(task_));  // Release

    delete this;
  }

 private:
  F task_;
};

}  // namespace exe::runtime::timer