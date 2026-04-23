#pragma once

#include <exe/runtime/task/task.hpp>
#include <exe/runtime/task/hint.hpp>

#include "fwd.hpp"
#include "work_stealing_queue.hpp"

#include <twist/ed/std/atomic.hpp>
#include <twist/ed/std/thread.hpp>
#include <twist/config.hpp>

#include <cstdlib>
#include <optional>
#include <random>
#include <span>

namespace exe::runtime::multi_thread::v2 {

class Worker {
 private:
  static constexpr size_t kLocalQueueCapacity =
      twist::config::kTwisted ? 7 : 256;
  static constexpr size_t kGlobalQueueRequestFrequency = 61;

 public:
  Worker(ThreadPool& host, size_t index);

  void Start();
  void Join();

  // Single producer
  void Push(task::TaskBase*, task::SchedulingHint);

  // Steal from this worker
  size_t StealTasks(std::span<task::TaskBase*> out_buffer);

  // Wake parked worker
  void Wake();

  static Worker* Current();

  ThreadPool* Host() const {
    return &host_;
  }

 private:
  // Use in Push
  void PushToLifoSlot(task::TaskBase* task);
  void PushToLocalQueue(task::TaskBase* task);
  void OffloadTasksToGlobalQueue(task::TaskBase* overflow);

  // Use in TryPickTask
  task::TaskBase* TryPickTaskFromLifoSlot();
  task::TaskBase* TryStealTasks();
  task::TaskBase* TryPickTaskFromGlobalQueue();
  task::TaskBase* TryGrabTasksFromGlobalQueue();

  task::TaskBase* PickTask();  // Or park thread

  // Run Loop
  void Work();

 private:
  ThreadPool& host_;
  const size_t index_;

  // Scheduling iteration
  size_t iter_ = 0;

  // Worker thread
  std::optional<twist::ed::std::thread> thread_;

  // Local queue
  WorkStealingTaskQueue<kLocalQueueCapacity> local_queue_;

  // LIFO slot
  task::TaskBase* lifo_slot_{nullptr};

  // Deterministic pseudo-randomness for work stealing
  std::mt19937_64 twister_;

  // Parking lot
  twist::ed::std::atomic<uint32_t> wakeups_{0};
  
};

}  // namespace exe::runtime::multi_thread::v2
