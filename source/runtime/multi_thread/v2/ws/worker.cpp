#include "worker.hpp"
#include "thread_pool.hpp"

#include <twist/ed/static/thread_local/ptr.hpp>
#include <twist/ed/wait/futex.hpp>

#include <twist/trace/log.hpp>

#include <algorithm>
#include <iostream>

#include <exe/debug/logging.hpp>

namespace exe::runtime::multi_thread::v2 {

TWISTED_STATIC_THREAD_LOCAL_PTR(Worker, current_worker_ptr);

Worker::Worker(ThreadPool& host, size_t index)
    : host_(host),
      index_(index) {
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
      break;
    case task::SchedulingHint::Next:
      PushToLifoSlot(task);
      break;
    case task::SchedulingHint::Yield:
      Host()->global_queue_.PushOne(task);
      break;
    default:
      assert(false && "Impossible scheduler hint");
  }
  Host()->coordinator_.NotifyOnSubmit();
}

size_t Worker::StealTasks(std::span<task::TaskBase*> out_buffer) {
  return local_queue_.Grab(out_buffer);
}

void Worker::Wake() {
  auto wake_key = twist::ed::futex::PrepareWake(wakeups_);
  wakeups_.fetch_add(1);
  twist::ed::futex::WakeOne(wake_key);
}

Worker* Worker::Current() {
  return current_worker_ptr;
}

void Worker::PushToLifoSlot(task::TaskBase* task) {
  if (lifo_slot_ == nullptr) {
    lifo_slot_ = task;
    return;
  }
  PushToLocalQueue(std::exchange(lifo_slot_, task));
}

void Worker::PushToLocalQueue(task::TaskBase* task) {
  if (!local_queue_.TryPush(task)) {
    OffloadTasksToGlobalQueue(task);
  }
}

void Worker::OffloadTasksToGlobalQueue(task::TaskBase* overflow) {
  std::array<task::TaskBase*, kGrabAmount> buffer;

  size_t got = local_queue_.Grab(std::span(buffer.data(), kGrabAmount - 1));
  buffer[got++] = overflow;

  Host()->global_queue_.PushMany(std::span(buffer.data(), got));
  Host()->coordinator_.NotifyOnSubmit();
}

task::TaskBase* Worker::GetFromBuffer(std::span<task::TaskBase*> buffer) {
  if (buffer.empty()) {
    return nullptr;
  }

  for (size_t _ = 1; _ < buffer.size(); ++_) {
    PushToLocalQueue(buffer[_]);
  }

  return buffer[0];
}

// Use in TryPickTask
task::TaskBase* Worker::TryPickTaskFromLifoSlot() {
  return std::exchange(lifo_slot_, nullptr);
}

task::TaskBase* Worker::TryStealTasks() {
  std::array<task::TaskBase*, kGrabAmount> buffer;
  size_t got = 0;

  // shuffle tasks
  // std::vector<Worker*> workers;
  // workers.reserve(Host()->num_threads_);
  // for (auto& w : Host()->workers_) {
  //   workers.push_back(&w);
  // }
  // std::shuffle(workers.begin(), workers.end(), twister_);

  // steal
  for (auto& worker : Host()->workers_) {
    if (&worker == this) {
      continue;
    }
    got = worker.StealTasks(buffer);
    if (got > 0) {
      break;
    }
  }

  LOG(" worker {} has stolen {} tasks", index_, got);

  return GetFromBuffer(std::span(buffer.data(), got));
}

task::TaskBase* Worker::TryPickTaskFromGlobalQueue() {
  return Host()->global_queue_.TryPopOne();
}

task::TaskBase* Worker::TryGrabTasksFromGlobalQueue() {
  std::array<task::TaskBase*, kGrabAmount> buffer;
  size_t got = Host()->global_queue_.Grab(buffer);
  LOG(" worker {} has grabbed {} tasks from global queue", index_, got);
  return GetFromBuffer(std::span(buffer.data(), got));
}

task::TaskBase* Worker::TryGetWork() {
  if (filo_chain_ < kFILOMaxChain) {  // FILO slot
    if (auto* task = TryPickTaskFromLifoSlot()) {
      ++filo_chain_;
      return task;
    }
  }
  filo_chain_ = 0;

  {  // local queue
    if (auto* task = local_queue_.TryPop()) {
      LOG(" worker {} has got task from local queue", index_);
      return task;
    }
  }

  {  // global queue
    if (auto* task = TryGrabTasksFromGlobalQueue()) {
      return task;
    }
  }

  {  // stealing
    if (auto* task = TryStealTasks()) {
      return task;
    }
  }

  {  // gc
  }

  return nullptr;
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

  {  // disable starvation (%61)
    if (++iter_ % 61 == 0) {
      if (auto* task = TryGrabTasksFromGlobalQueue()) {
        return task;
      }
    }
  }

  while (true) {
    if (auto* task = TryGetWork()) {
      return task;
    }

    uint32_t expected_wu = wakeups_.load();
    Host()->coordinator_.AddStoppedWorker(this);

    if (auto* task = TryGetWork()) {
      Host()->coordinator_.Unlink(this);
      return task;
    }

    if (has_stopped_.load()) {
      Host()->coordinator_.Unlink(this);
      return nullptr;
    }

    LOG("worker {} add stopped, wakeups={}", index_, expected_wu);
    LOG("worker {} park", index_);
    twist::ed::futex::Wait(wakeups_, expected_wu);
    LOG("worker {} wake from futex, wakeups={}", index_, wakeups_.load());

    Host()->coordinator_.Unlink(this);
  }

  // return nullptr;
}

void Worker::Work() {
  current_worker_ptr = this;

  while (task::TaskBase* next = PickTask()) {
    LOG(" worker {} is running", index_);
    Host()->coordinator_.NotifyOnSubmit();
    next->Run();
  }

  current_worker_ptr = nullptr;  // ???
}

// void Worker::Park(size_t expected) {
//   // std::cout << "go to sleep" + std::to_string(index_) + "\n";
//   std::ignore = expected;
//   Host()->coordinator_.AddStoppedWorker(this);
//   // if worker don't park, coordinator incrimented wakeups_ => whis worker is
//   out of it's queue

//   // std::cout << "waked" + std::to_string(index_) + "\n";
// }

void Worker::Stop() {
  has_stopped_.store(true);
  Wake();
}

}  // namespace exe::runtime::multi_thread::v2