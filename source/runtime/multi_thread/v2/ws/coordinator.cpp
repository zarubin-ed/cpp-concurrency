#include "coordinator.hpp"

namespace exe::runtime::multi_thread::v2 {

bool Coordinator::ShouldWakeWorker() const {
  return true;
}

void Coordinator::AddStoppedWorker(Worker* w) {
  std::lock_guard guard(spin_);
  if (!w->IsLinked()) {
    stopped_.PushBack(w);
  }
}

void Coordinator::WakeWorker() {
  LOG("wake requested");
  std::lock_guard guard(spin_);
  if (!stopped_.IsEmpty()) {
    auto w = stopped_.PopFrontNonEmpty();
    w->Wake();
    LOG("wake worker {}", w->Index());
  } else {
    LOG("wake requested, no stopped workers");
  }
}

void Coordinator::Unlink(Worker* w) {
  std::lock_guard guard(spin_);
  if (w->IsLinked()) {
    w->Unlink();
  }
}
}  // namespace exe::runtime::multi_thread::v2