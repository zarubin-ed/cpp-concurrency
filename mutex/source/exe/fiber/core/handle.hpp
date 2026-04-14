#pragma once

#include "fwd.hpp"

#include <chrono>

namespace exe::fiber {

// Opaque non-owning handle to a _suspended_ fiber
// Trivially copyable

class FiberHandle {
  friend class Fiber;

 public:
  FiberHandle()
      : FiberHandle(nullptr) {
  }

  static FiberHandle Invalid() {
    return FiberHandle(nullptr);
  }

  bool IsValid() const {
    return fiber_ != nullptr;
  }

  // Resumes execution of the fiber in current task
  // One-shot
  void Resume();

  void Schedule();
  void Schedule(std::chrono::microseconds);

 private:
  explicit FiberHandle(Fiber* fiber)
      : fiber_(fiber) {
  }

  Fiber* Release();

 private:
  Fiber* fiber_;
};

}  // namespace exe::fiber
