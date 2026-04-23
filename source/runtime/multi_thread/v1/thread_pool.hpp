#pragma once

#include <exe/runtime/task/task.hpp>
#include <exe/runtime/task/scheduler.hpp>

#include "queue.hpp"

#include <twist/ed/std/thread.hpp>
#include <twist/ed/static/thread_local/ptr.hpp>

#include <cstddef>
#include <vector>

namespace exe::runtime::multi_thread {

// Fixed-size pool of worker threads

class ThreadPool final : public task::IScheduler {
  using TaskBase = exe::runtime::task::TaskBase;

 public:
  explicit ThreadPool(size_t);
  ~ThreadPool();

  // Non-copyable
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  // Non-movable
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  void Start();

  void Submit(TaskBase*, task::SchedulingHint) override;

  static ThreadPool* Current();

  void Stop();

 private:
  void Work();

  UnboundedBlockingQueue<TaskBase> tasks_;
  std::vector<twist::ed::std::thread> workers_;
  const size_t num_workers_;
  bool has_stopped_ = false;
};

};  // namespace exe::runtime::multi_thread