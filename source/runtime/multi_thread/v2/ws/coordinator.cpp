#include "coordinator.hpp"

namespace exe::runtime::multi_thread::v2 {

bool Coordinator::ShouldWakeWorker() const {
  return head_.load() < tail_.load(); // Not implemented
}

void Coordinator::WakeWorker() {
  auto w = workers_[head_.fetch_add(1) % workers_.size()];
  w->Wake();
}

void Coordinator::AddStoppedWorker(Worker* w) {
  workers_[tail_.fetch_add(1) % workers_.size()] = w;
}

}  // namespace exe::runtime::multi_thread::v2
