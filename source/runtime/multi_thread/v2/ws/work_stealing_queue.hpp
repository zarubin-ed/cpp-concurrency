#pragma once

#include <exe/runtime/task/task.hpp>

#include <twist/ed/std/atomic.hpp>

#include <twist/assist/preempt.hpp>

#include <array>
#include <span>

namespace exe::runtime::multi_thread::v2 {

template <size_t Capacity>
class WorkStealingTaskQueue {
  using Task = task::TaskBase;
  struct Slot {
    Task* Load() {
      return task_.load();
    }

    void Store(Task* task) {
      task_.store(task);
    }

   private:
    twist::ed::std::atomic<Task*> task_{nullptr};
  };

 public:
  bool TryPush(Task* task) {
    size_t tail = tail_.load();
    size_t head = head_.load();
    if (head + Capacity <= tail) {
      return false;
    }

    buffer_[Mod(tail)].Store(task);
    tail_.store(tail + 1);
    return true;
  }

  Task* TryPop() {
    size_t head;
    size_t tail;
    Task* task;
    do {
      head = head_.load();
      tail = tail_.load();
      if (head == tail) {
        return nullptr;
      }
      task = buffer_[Mod(head)].Load();
    } while (!head_.compare_exchange_weak(head, head + 1));
    return task;
  }

  size_t Grab(std::span<Task*> out_buffer) {
    size_t head;
    size_t tail;
    size_t i;
    do {
      head = head_.load();
      tail = tail_.load();
      for (i = head; i < tail && i - head < out_buffer.size(); ++i) {
        out_buffer[i - head] = buffer_[Mod(i)].Load();
      }
    } while (!head_.compare_exchange_weak(head, i));
    return i - head;
  }

  size_t SpaceLowerBound() const {
    return Capacity - (tail_.load() - head_.load());
  }

 private:
  size_t Mod(size_t num) {
    if constexpr ((Capacity & (Capacity - 1)) == 0) {
      return num & (Capacity - 1);
    } else {
      return num % Capacity;
    }
  }

  std::array<Slot, Capacity> buffer_;
  twist::ed::std::atomic<size_t> head_{0};
  twist::ed::std::atomic<size_t> tail_{0};
};

}  // namespace exe::runtime::multi_thread::v2

// considers parking
// //