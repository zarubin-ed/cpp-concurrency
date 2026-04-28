#include "thread_pool.hpp"

#include <twist/ed/static/thread_local/ptr.hpp>

#include <iostream>  // to delete

namespace exe::runtime::multi_thread::v2 {

// TWISTED_STATIC_THREAD_LOCAL_PTR(ThreadPool, current_pool_ptr);

ThreadPool::ThreadPool(size_t num_threads)
    : num_threads_(num_threads),
      coordinator_(num_threads) {
}

void ThreadPool::Start() {
  for (size_t i = 0; i < num_threads_; ++i) {
    workers_.emplace_back(*this, i);
  }

  for (auto& worker : workers_) {
    worker.Start();
  }
}

ThreadPool::~ThreadPool() {
  // Not implemente
}

// void ThreadPool::Destribute(task::TaskBase* task) {
//   workers_[workers_queue_head_.fetch_add(1) % num_threads_].Push(task);
// }

void ThreadPool::Submit(task::TaskBase* task, task::SchedulingHint hint) {
  if (Current() != this) {
    LOG("external submit -> global");
    global_queue_.PushOne(task);
    coordinator_.NotifyOnSubmit();
  } else {
    LOG("internal submit");
    Worker::Current()->Push(task, hint);
  }
}

void ThreadPool::Stop() {
  for (auto& worker : workers_) {
    worker.Stop();
  }
  for (auto& worker : workers_) {
    worker.Join();
  }
}

ThreadPool* ThreadPool::Current() {
  return Worker::Current() == nullptr ? nullptr : Worker::Current()->Host();
}

}  // namespace exe::runtime::multi_thread::v2
