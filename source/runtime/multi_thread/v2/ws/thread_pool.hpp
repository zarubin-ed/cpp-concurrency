#pragma once

#include <exe/runtime/task/scheduler.hpp>

#include "global_queue.hpp"
#include "worker.hpp"
#include "coordinator.hpp"

// std::random_device
#include <twist/ed/std/random.hpp>

#include <cstddef>
#include <deque>

namespace exe::runtime::multi_thread::v2 {

// Scalable work-stealing scheduler for
// fibers, stackless coroutines and futures

class ThreadPool : public task::IScheduler {
  friend class Worker;

 public:
  explicit ThreadPool(size_t num_threads);
  ~ThreadPool();

  // Non-copyable
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  // Non-movable
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  void Start();

  // task::IScheduler
  void Submit(task::TaskBase*, task::SchedulingHint) override;

  void Stop();

  static ThreadPool* Current();

 private:
  const size_t num_threads_;
  std::deque<Worker> workers_;
  Coordinator coordinator_;
  GlobalTaskQueue global_queue_;
  twist::ed::std::random_device random_device_;
};

}  // namespace exe::runtime::multi_thread::v2
