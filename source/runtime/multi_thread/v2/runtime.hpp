#pragma once

#include <exe/runtime/view.hpp>

#include "ws/thread_pool.hpp"

namespace exe::runtime::multi_thread::v2 {

class Runtime {
 public:
  explicit Runtime(size_t /*num_workers*/);

  // NOLINTNEXTLINE
  operator View() {
    return {nullptr, nullptr};  // Not implemented
  }

  Runtime& WithTimers() {
    // Not implemented
    return *this;
  }

  void Start();
  void Stop();

  bool Here() const;

 private:
  //
};

}  // namespace exe::runtime::multi_thread::v2
