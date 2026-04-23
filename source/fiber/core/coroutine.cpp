#include "coroutine.hpp"

namespace exe::fiber {

Coroutine::Coroutine(Body routine)
    : routine_(std::move(routine)) {
}

void Coroutine::Resume() {
  routine_.Resume();
}

void Coroutine::Suspend() {
  routine_.Suspend();
}

bool Coroutine::IsDone() const {
  return routine_.is_done_;
}

}  // namespace exe::fiber