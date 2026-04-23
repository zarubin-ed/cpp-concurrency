#include "thread_pool.hpp"

#include <twist/ed/static/thread_local/ptr.hpp>

namespace exe::runtime::multi_thread {

TWISTED_STATIC_THREAD_LOCAL_PTR(ThreadPool, current_pool_ptr);

ThreadPool::ThreadPool(size_t num_workers)
    : num_workers_(num_workers) {
}

void ThreadPool::Start() {
  for (size_t _ = 0; _ < num_workers_; ++_) {
    workers_.emplace_back(&ThreadPool::Work, this);
  }
}

ThreadPool::~ThreadPool() {
  assert(has_stopped_);
}

ThreadPool* ThreadPool::Current() {
  return current_pool_ptr;
}

void ThreadPool::Submit(TaskBase* task, task::SchedulingHint) {
  tasks_.Push(std::move(task));
}

void ThreadPool::Stop() {
  tasks_.Close();
  has_stopped_ = true;
  for (auto& worker : workers_) {
    worker.join();
  }
}

void ThreadPool::Work() {
  current_pool_ptr = this;
  while (auto task = tasks_.Pop()) {
    (*task)->Run();
  }
}

};  // namespace exe::runtime::multi_thread