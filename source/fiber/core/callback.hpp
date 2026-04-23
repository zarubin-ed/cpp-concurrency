#pragma once

#include "handle.hpp"

#include <cassert>

namespace exe::fiber {

class Callback {
  template <typename F>
  struct Wrapper {
    static void Invoker(void* fptr, FiberHandle handle) {
      std::invoke(*static_cast<F*>(fptr), handle);
    }
  };

 public:
  void operator()(FiberHandle handle) {
    assert(operator bool());
    invoker_(callback_, handle);
  }

  template <typename F>
  explicit Callback(F& callback)
      : callback_(&callback),
        invoker_(&Wrapper<F>::Invoker) {
  }

  Callback() = default;

  explicit operator bool() const {
    return callback_ != nullptr;
  }

 private:
  void* callback_ = nullptr;
  void (*invoker_)(void*, FiberHandle) = nullptr;
};

}  // namespace exe::fiber