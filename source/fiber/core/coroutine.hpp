#pragma once

#include "body.hpp"
#include "stack.hpp"

#include <twist/ed/sure/context.hpp>

namespace exe::fiber {

constexpr size_t kStackSize = 16 * 1024;

class Coroutine {
 public:
  class SuspendContext {
    friend class Coroutine;

   public:
    void Suspend() {
      self_->Suspend();
    }

   private:
    explicit SuspendContext(Coroutine* coro)
        : self_(coro) {
    }

   private:
    Coroutine* self_;
  };

 public:
  explicit Coroutine(Body);

  void Resume();

  void Suspend();

  bool IsDone() const;

 private:
  class Routine : public sure::ITrampoline {
   public:
    explicit Routine(Body routine)
        : routine_(std::move(routine)),
          stack_(Stack::AllocateAtLeastBytes(kStackSize)) {
      routine_context_.Setup(stack_.MutView(), this);
    }

    void Run() noexcept override {
      routine_();

      is_done_ = true;
      routine_context_.ExitTo(saved_context_);
    }

   private:
    void Resume() {
      saved_context_.SwitchTo(routine_context_);
    }

    void Suspend() {
      routine_context_.SwitchTo(saved_context_);
    }

    friend class Coroutine;

    Body routine_;

    Stack stack_;
    twist::ed::sure::ExecutionContext routine_context_;
    twist::ed::sure::ExecutionContext saved_context_;
    bool is_done_ = false;
  };

  Routine routine_;
};

}  // namespace exe::fiber