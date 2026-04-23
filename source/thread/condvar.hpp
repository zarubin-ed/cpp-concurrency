#pragma once

#include <twist/ed/std/atomic.hpp>
#include <twist/ed/wait/futex.hpp>

#include <cstdint>

namespace primitives {

class CondVar {
 public:
  // Mutex - BasicLockable
  // https://en.cppreference.com/w/cpp/named_req/BasicLockable
  template <class Mutex>
  void Wait(Mutex& mutex) {
    size_t my_order = atom_.load();
    mutex.unlock();
    while (atom_.load() <= my_order) {
      twist::ed::futex::Wait(atom_, my_order);
    }
    mutex.lock();
  }

  void NotifyOne() {
    auto wake_key = twist::ed::futex::PrepareWake(atom_);
    atom_.fetch_add(1);
    twist::ed::futex::WakeOne(wake_key);
  }

  void NotifyAll() {
    auto wake_key = twist::ed::futex::PrepareWake(atom_);
    atom_.fetch_add(1);
    twist::ed::futex::WakeAll(wake_key);
  }

  template <class Mutex>
  void wait(Mutex& mutex) {  // NOLINT
    Wait(mutex);
  }

  void notify_one() {  // NOLINT
    NotifyOne();
  }

  void notify_all() {  // NOLINT
    NotifyAll();
  }

 private:
  twist::ed::std::atomic<uint32_t> atom_{0};
};

};  // namespace primitives
