#pragma once

#include "body.hpp"
#include "coroutine.hpp"
#include "callback.hpp"

#include <exe/runtime/view.hpp>

#include <twist/ed/std/atomic.hpp>

namespace exe::fiber {

// Fiber = Stackful coroutine x Scheduler

class Fiber {
  friend class FiberHandle;

 public:
  Fiber(runtime::View, Body);

  static Fiber& Self();

  void Go(Body);

  void Resume();

  void Suspend(Callback);

  void Schedule();

 private:
  Callback ConsumeCallback();

  Callback cb_;

  runtime::View runtime_;
  Coroutine coroutine_;
};

}  // namespace exe::fiber