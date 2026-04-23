#pragma once
#include "worker.hpp"

#include <twist/ed/std/atomic.hpp>

#include <cstddef>
#include <deque>

namespace exe::runtime::multi_thread::v2 {

class Coordinator {
 public:
  explicit Coordinator(size_t num_threads) : workers_(num_threads, nullptr) {
    workers_.shrink_to_fit(); // ???
  }

  void NotifyOnSubmit() {
    if (ShouldWakeWorker()) {
      WakeWorker();
    }
  }

  void AddStoppedWorker(Worker*);

 private:
  bool ShouldWakeWorker() const;
  void WakeWorker();

 private:
  std::vector<Worker*> workers_;
  twist::ed::std::atomic_size_t head_{0};
  twist::ed::std::atomic_size_t tail_{0};
};

}  // namespace exe::runtime::multi_thread::v2
