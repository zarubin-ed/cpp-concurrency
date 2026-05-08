#include "coordinator.hpp"

namespace exe::runtime::multi_thread::v2 {

bool Coordinator::ShouldWakeWorker() const {
  return approximate_size_.load() > 0;
}

void Coordinator::AddStoppedWorker(Worker* w) {
  std::lock_guard guard(spin_);
  if (!w->IsLinked()) {
    approximate_size_.fetch_add(1);
    stopped_.PushBack(w);
  }
}

void Coordinator::AddSpinning() {
  spinning_.fetch_add(1);
}

void Coordinator::RemoveSpinning() {
  spinning_.fetch_sub(1);
}

void Coordinator::WakeWorker() {
  std::lock_guard guard(spin_);
  if (!stopped_.IsEmpty()) {
    auto w = stopped_.PopFrontNonEmpty();
    w->Wake();
    approximate_size_.fetch_sub(1);
  } else {
  }
}

void Coordinator::Unlink(Worker* w) {
  std::lock_guard guard(spin_);
  if (w->IsLinked()) {
    w->Unlink();
  }
}
}  // namespace exe::runtime::multi_thread::v2