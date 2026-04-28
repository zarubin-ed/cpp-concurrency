#pragma once
#include "worker.hpp"

#include <exe/thread/spinlock.hpp>

#include <twist/ed/std/atomic.hpp>
#include <twist/ed/std/mutex.hpp>

#include <cstddef>
#include <deque>
#include <queue>

#include <exe/debug/logging.hpp>

namespace exe::runtime::multi_thread::v2 {

class Coordinator {
 public:
  explicit Coordinator(size_t num_threads)
      : num_threads_(num_threads) {
    // workers_.shrink_to_fit(); // ???
  }

  void NotifyOnSubmit() {
    LOG("notify on submit");
    if (ShouldWakeWorker()) {
      WakeWorker();
    }
  }

  void AddStoppedWorker(Worker*);

  void Unlink(Worker* w);

 private:
  bool ShouldWakeWorker() const;
  void WakeWorker();

 private:
  struct WorkerSlot {
    WorkerSlot()
        : worker_(nullptr) {
    }

    Worker* Load() const {
      return worker_.load();
    }

    void Store(Worker* worker) {
      worker_.store(worker);
    }

    twist::ed::std::atomic<Worker*> worker_;
  };

  [[maybe_unused]] size_t num_threads_;
  vvv::IntrusiveList<Worker> stopped_;

  thread::SpinLock spin_;
};

}  // namespace exe::runtime::multi_thread::v2
