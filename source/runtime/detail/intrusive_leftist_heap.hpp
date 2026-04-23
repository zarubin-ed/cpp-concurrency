#pragma once

#include <exe/runtime/timer/timer.hpp>

namespace exe::runtime::detail {

class IntrusiveTimerLeftistHeap {
  using TimerBase = timer::TimerBase;

 public:
  void Push(TimerBase* item) {
    item->prev = nullptr;
    item->next = nullptr;
    item->depth = 1;
    top_ = Merge(top_, item);
  }

  static size_t Depth(TimerBase* node) {
    return node != nullptr ? node->depth : 0;
  }

  TimerBase* Merge(TimerBase* lhs, TimerBase* rhs) {
    if (lhs == nullptr) {
      return rhs;
    }
    if (rhs == nullptr) {
      return lhs;
    }

    if (lhs->deadline > rhs->deadline) {
      std::swap(lhs, rhs);
    }

    lhs->next = Merge(static_cast<TimerBase*>(lhs->next), rhs);

    if (Depth(static_cast<TimerBase*>(lhs->prev)) <
        Depth(static_cast<TimerBase*>(lhs->next))) {
      std::swap(lhs->prev, lhs->next);
    }

    lhs->depth = Depth(static_cast<TimerBase*>(lhs->next)) + 1;
    return lhs;
  }

  TimerBase* Top() {
    return top_;
  }

  const TimerBase* Top() const {
    return top_;
  }

  TimerBase* Pop() {
    auto* ret = top_;
    top_ = Merge(static_cast<TimerBase*>(ret->prev),
                 static_cast<TimerBase*>(ret->next));

    ret->prev = nullptr;
    ret->next = nullptr;
    return ret;
  }

  bool IsEmpty() const {
    return top_ == nullptr;
  }

 private:
  TimerBase* top_ = nullptr;
};

}  // namespace exe::runtime::detail
