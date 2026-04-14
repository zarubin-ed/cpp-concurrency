#include "wait_queue.hpp"

#include <exe/fiber/core/fiber.hpp>

#include <queue>

namespace exe::fiber {

void WaitQueue::WakeOne() {
  fibers_.front().Schedule();
  fibers_.pop();
}

void WaitQueue::WakeAll() {
  while (!IsEmpty()) {
    WakeOne();
  }
}

bool WaitQueue::IsEmpty() {
  return fibers_.empty();
}

}  // namespace exe::fiber
