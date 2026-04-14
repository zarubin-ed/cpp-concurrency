#include "runtime.hpp"

namespace exe::runtime::multi_thread {

Runtime::Runtime(size_t num_workers)
    : tasks_(num_workers),
      timer_(&tasks_) {
}

void Runtime::Start() {
  tasks_.Start();
  if (timer_.IsTimerEnabled()) {
    timer_.Start();
  }
}

void Runtime::Stop() {
  tasks_.Stop();
  if (timer_.IsTimerEnabled()) {
    timer_.Stop();
  }
}

bool Runtime::Here() const {
  return tasks_.Current() == &tasks_;
}

Runtime& Runtime::WithTimers() {
  timer_.EnableTimer();
  return *this;
}

}  // namespace exe::runtime::multi_thread
