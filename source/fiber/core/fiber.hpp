#pragma once

#include "body.hpp"
#include "coroutine.hpp"
#include "callback.hpp"

#include <exe/runtime/view.hpp>
#include <exe/runtime/task/task.hpp>

#include <twist/ed/std/atomic.hpp>

namespace exe::fiber {

// Fiber = Stackful coroutine x Scheduler

class Fiber final : public exe::runtime::task::TaskBase {
  friend class FiberHandle;

 public:
  Fiber(runtime::View, Body);

  static Fiber& Self();

  static void Spawn(runtime::View runtime, Body func);

  void Go(Body);

  void Resume();

  void Suspend(Callback);

  void Run() noexcept override;

 private:
  Callback cb_;

  runtime::View runtime_;
  Coroutine coroutine_;
};

}  // namespace exe::fiber