#pragma once
#include "worker.hpp"

#include <exe/thread/spinlock.hpp>

#include <twist/ed/std/atomic.hpp>
#include <twist/ed/std/mutex.hpp>

#include <cstddef>
#include <deque>
#include <queue>

namespace exe::runtime::multi_thread::v2 {

class Coordinator {
 public:
  explicit Coordinator(size_t num_threads)
      : num_threads_(num_threads) {
    // workers_.shrink_to_fit(); // ???
  }

  void NotifyOnSubmit() {
    if (ShouldWakeWorker()) {
      WakeWorker();
    }
  }

  void AddStoppedWorker(Worker*);
  void AddSpinning();
  void RemoveSpinning();

  void Unlink(Worker* w);

 private:
  bool ShouldWakeWorker() const;
  void WakeWorker();

 private:
  [[maybe_unused]] size_t num_threads_;
  vvv::IntrusiveList<Worker> stopped_;
  twist::ed::std::atomic<uint64_t> approximate_size_{0};
  twist::ed::std::atomic<uint64_t> spinning_{0};
  thread::SpinLock spin_;
};

}  // namespace exe::runtime::multi_thread::v2
