#include "wait_queue.hpp"

namespace exe::fiber {

void WaitQueue::WakeOne() {
  if (IsEmpty()) {
    return;
  }

  fibers_.PopFrontNonEmpty()->GetHandle().Schedule();
}

void WaitQueue::WakeAll() {
  while (!IsEmpty()) {
    WakeOne();
  }
}

bool WaitQueue::IsEmpty() {
  return fibers_.IsEmpty();
}

}  // namespace exe::fiber
