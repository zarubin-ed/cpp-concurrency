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

  ConsumeCallback()(FiberHandle(this));
}

Fiber::Fiber(runtime::View runtime, Body func)
    : runtime_(runtime),
      coroutine_(std::move(func)) {
  FiberHandle(this).Schedule();
}

void Fiber::Go(Body func) {
  new Fiber(runtime_, std::move(func));
}

Callback Fiber::ConsumeCallback() {
  assert(cb_);
  return std::exchange(cb_, {});
}

void Fiber::Suspend(Callback cb) {
  cb_ = std::move(cb);
  coroutine_.Suspend();
}

Fiber& Fiber::Self() {
  return *this_ptr;
}

}  // namespace exe::fiber