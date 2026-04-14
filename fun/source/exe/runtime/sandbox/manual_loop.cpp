#include "manual_loop.hpp"

namespace exe::runtime::sandbox {

void ManualLoop::Submit(task::Task task) {
  task_queue_.push_back(std::move(task));
}

// Run tasks

size_t ManualLoop::RunAtMostTasks(size_t limit) {
  size_t count = 0;
  while (count++ < limit && !task_queue_.empty()) {
    task_queue_.front()();
    task_queue_.pop_front();
  }
  return count - 1;
}

size_t ManualLoop::RunTasks() {
  return RunAtMostTasks(-1);
}

bool ManualLoop::IsEmpty() const {
  return task_queue_.empty();
}

}  // namespace exe::runtime::sandbox