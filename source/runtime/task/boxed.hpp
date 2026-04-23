#pragma once

#include "task.hpp"

#include <functional>
#include <utility>

namespace exe::runtime::task {

template <typename F>
class Boxed final : public TaskBase {
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

}  // namespace exe::runtime::task