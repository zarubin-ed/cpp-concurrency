#pragma once

#include "wait_group.hpp"

namespace exe::fiber {

// One-shot

class Event {
 public:
  Event() {
    wg_.Add(1);
  }

  void Wait() {
    wg_.Wait();
  }

  void Fire() {
    wg_.Done();
  }

 private:
  WaitGroup wg_;
};

}  // namespace exe::fiber
