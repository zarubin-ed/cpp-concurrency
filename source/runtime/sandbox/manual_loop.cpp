#include "manual_loop.hpp"

namespace exe::runtime::sandbox {

void ManualLoop::Submit(TaskBase* task) {
  task_queue_.PushBack(task);
}

// Run tasks

size_t ManualLoop::RunAtMostTasks(size_t limit) {
  size_t count = 0;
  while (count++ < limit && !task_queue_.IsEmpty()) {
    task_queue_.PopFrontNonEmpty()->Run();
  }
  return count - 1;
}

size_t ManualLoop::RunTasks() {
  return RunAtMostTasks(-1);
}

bool ManualLoop::IsEmpty() const {
  return task_queue_.IsEmpty();
}

}  // namespace exe::runtime::sandbox