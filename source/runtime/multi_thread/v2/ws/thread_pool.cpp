#include "thread_pool.hpp"

#include <cstdlib>  // std::abort

namespace exe::runtime::multi_thread::v2 {

ThreadPool::ThreadPool(size_t num_threads)
    : num_threads_(num_threads),
      coordinator_(num_threads) {
  // Not implemented
}

void ThreadPool::Start() {
  // Not implemented
}

ThreadPool::~ThreadPool() {
  // Not implemented
}

void ThreadPool::Submit(task::TaskBase* /*task*/,
                        task::SchedulingHint /*hint*/) {
  // Not implemented
}

void ThreadPool::Stop() {
  for (auto& worker : workers_) {
    worker.Join();
  }
}

ThreadPool* ThreadPool::Current() {
  return nullptr;  // Not implemented
}

}  // namespace exe::runtime::multi_thread::v2
