#pragma once

#include <exe/runtime/task/task.hpp>
#include <exe/runtime/task/scheduler.hpp>

#include <cstddef>
#include <deque>

namespace exe::runtime::sandbox {

// Single-threaded task queue

class ManualLoop final : public task::IScheduler {
  using TaskBase = task::TaskBase;

 public:
  ManualLoop() = default;

  // Non-copyable
  ManualLoop(const ManualLoop&) = delete;
  ManualLoop& operator=(const ManualLoop&) = delete;

  // Non-movable
  ManualLoop(ManualLoop&&) = delete;
  ManualLoop& operator=(ManualLoop&&) = delete;

  void Submit(TaskBase*,
              task::SchedulingHint = task::SchedulingHint::UpToYou) override;

  // Run tasks

  // Run at most `limit` tasks from queue
  // Returns number of completed tasks
  size_t RunAtMostTasks(size_t /* limit */);

  // Run next task if queue is not empty
  bool RunNextTask() {
    return RunAtMostTasks(1) == 1;
  }

  // Run tasks until queue is empty
  // Returns number of completed tasks
  // Post-condition: IsEmpty() == true
  size_t RunTasks();

  bool IsEmpty() const;

  bool NonEmpty() const {
    return !IsEmpty();
  }

 private:
  vvv::IntrusiveList<TaskBase> task_queue_;
};

}  // namespace exe::runtime::sandbox