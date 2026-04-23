#include "fiber.hpp"
#include "callback.hpp"

#include <twist/ed/static/thread_local/ptr.hpp>

namespace exe::fiber {

TWISTED_STATIC_THREAD_LOCAL_PTR(Fiber, this_ptr);

void Fiber::Resume() {
  this_ptr = this;
  coroutine_.Resume();
  this_ptr = nullptr;

  if (coroutine_.IsDone()) {
    delete this;
    return;
  }

  cb_(FiberHandle(this));
}

void Fiber::Run() noexcept {
  Resume();
}

Fiber::Fiber(runtime::View runtime, Body func)
    : runtime_(runtime),
      coroutine_(std::move(func)) {
}

void Fiber::Spawn(runtime::View runtime, Body func) {
  auto* fiber = new Fiber(runtime, std::move(func));
  FiberHandle(fiber).Schedule();
}

void Fiber::Go(Body func) {
  Spawn(runtime_, std::move(func));
}

void Fiber::Suspend(Callback cb) {
  cb_ = cb;
  coroutine_.Suspend();
}

Fiber& Fiber::Self() {
  return *this_ptr;
}

}  // namespace exe::fiber