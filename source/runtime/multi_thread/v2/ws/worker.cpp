#include "worker.hpp"
#include "thread_pool.hpp"

#include <twist/ed/static/thread_local/ptr.hpp>

namespace exe::runtime::multi_thread::v2 {

TWISTED_STATIC_THREAD_LOCAL_PTR(Worker, current_worker_ptr);

Worker::Worker(ThreadPool& host, size_t index)
    : host_(host),
      index_(index) {
  // Not implemented
}

void Worker::Start() {
  thread_.emplace([this] {
    Work();
  });
}

void Worker::Join() {
  thread_->join();
}

void Worker::Push(task::TaskBase* task, task::SchedulingHint hint) {
  switch (hint) {
    case task::SchedulingHint::UpToYou:
      PushToLocalQueue(task);
      return;
    case task::SchedulingHint::Next:
      PushToLifoSlot(task);
      return;
    case task::SchedulingHint::Yield:
      OffloadTasksToGlobalQueue(task);
      return;
    default:
      assert(false && "Impossible scheduler hint");
  }
}

void Worker::Wake() {
  wakeups_.fetch_add(1);
  wakeups_.notify_one();
}

Worker* Worker::Current() {
  return current_worker_ptr;
}

void Worker::PushToLifoSlot(task::TaskBase* task) {

}
void Worker::PushToLocalQueue(task::TaskBase* task) {

}
void Worker::OffloadTasksToGlobalQueue(task::TaskBase* overflow) {

}

// Use in TryPickTask
task::TaskBase* Worker::TryPickTaskFromLifoSlot() {

}

task::TaskBase* Worker::TryStealTasks() {
  // host_.coordinator_.
}

task::TaskBase* Worker::TryPickTaskFromGlobalQueue() {
  return Host()->global_queue_.TryPopOne();
}

task::TaskBase* Worker::TryGrabTasksFromGlobalQueue() {

}

task::TaskBase* Worker::PickTask() {
  // Poll in order:
  // * [%61] global queue
  // * LIFO slot
  // * local queue
  // * global queue
  // * work stealing
  // then
  //   park worker

  task::TaskBase* desired_task = nullptr;

  if (++iter_ % 61 == 0) { // global queue
    desired_task = TryPickTaskFromGlobalQueue();
    if (desired_task != nullptr) {
      return desired_task;
    }
  }

  if (1 == 0) { // FILO slot
    desired_task = TryPickTaskFromLifoSlot();
    if (desired_task != nullptr) {
      return desired_task;
    }
  }

  { // local queue
    desired_task = local_queue_.TryPop();
    if (desired_task != nullptr) {
      return desired_task;
    }
  }

  { // global queue
    desired_task = TryGrabTasksFromGlobalQueue();
    if (desired_task != nullptr) {
      return desired_task;
    }
  }

  { // stealing
    desired_task = TryStealTasks();
    if (desired_task != nullptr) {
      return desired_task;
    }
  }

  { // gc

  }

  wakeups_.wait(wakeups_.load());

  return nullptr;  // Not implemented
}

void Worker::Work() {
  current_worker_ptr = this;

  while (task::TaskBase* next = PickTask()) {
    next->Run();
  }

  current_worker_ptr = nullptr; // ???
}

}  // namespace exe::runtime::multi_thread::v2
