#pragma once

#include <exe/runtime/task/task.hpp>
#include <exe/runtime/task/hint.hpp>

#include "fwd.hpp"
#include "work_stealing_queue.hpp"

#include <twist/ed/std/atomic.hpp>
#include <twist/ed/std/thread.hpp>
#include <twist/config.hpp>

#include <vvv/list.hpp>

#include <cstdlib>
#include <optional>
#include <random>
#include <span>

namespace exe::runtime::multi_thread::v2 {

class Worker : public vvv::IntrusiveListNode<Worker> {
 private:
  static constexpr size_t kLocalQueueCapacity =
      twist::config::kTwisted ? 7 : 256;
  static constexpr size_t kFILOMaxChain = 16;
  static constexpr size_t kGrabAmount = std::max(1ul, kLocalQueueCapacity / 2);

 public:
  Worker(ThreadPool& host, size_t index);

  void Start();
  void Stop();
  void Join();

  // Single producer
  void Push(task::TaskBase*,
            task::SchedulingHint = task::SchedulingHint::UpToYou);

  // Steal from this worker
  size_t StealTasks(std::span<task::TaskBase*> out_buffer);

  // Wake parked worker
  void Wake();

  static Worker* Current();

  ThreadPool* Host() const {
    return &host_;
  }

  size_t Index() const {
    return index_;
  }

 private:
  // Use in Push
  void PushToLifoSlot(task::TaskBase* task);
  void PushToLocalQueue(task::TaskBase* task);
  void OffloadTasksToGlobalQueue(task::TaskBase* overflow);

  // Use in TryPickTask
  task::TaskBase* GetFromBuffer(std::span<task::TaskBase*>);
  task::TaskBase* TryGetWork();

  task::TaskBase* TryPickTaskFromLifoSlot();
  task::TaskBase* TryStealTasks();
  task::TaskBase* TryPickTaskFromGlobalQueue();
  task::TaskBase* TryGrabTasksFromGlobalQueue();

  task::TaskBase* PickTask();  // Or park thread

  // Run Loop
  void Work();

  void Park(size_t expected);

 private:
  ThreadPool& host_;
  [[maybe_unused]] const size_t index_;  // useless, maybe for debug

  // Scheduling iteration
  [[maybe_unused]] size_t iter_ = 0;

  // Worker thread
  std::optional<twist::ed::std::thread> thread_;

  // Local queue
  WorkStealingTaskQueue<kLocalQueueCapacity> local_queue_;

  // LIFO slot
  task::TaskBase* lifo_slot_{nullptr};
  size_t filo_chain_ = 0;

  // Deterministic pseudo-randomness for work stealing
  std::mt19937_64 twister_;

  // Parking lot
  twist::ed::std::atomic<uint32_t> wakeups_{0};

  // Stop condition
  twist::ed::std::atomic<bool> has_stopped_{false};
};

}  // namespace exe::runtime::multi_thread::v2
