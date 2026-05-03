#pragma once

#include <twist/ed/std/atomic.hpp>

namespace gc::hazard {

struct PtrRecord {
  PtrRecord() = default;

  void Set(void* obj) {
    ptr_.store(obj);
  }

  void* Get() {
    return ptr_.load();
  }

  void Reset() {
    ptr_.store(nullptr);
  }

 private:
  // Actual hazard pointer
  twist::ed::std::atomic<void*> ptr_{nullptr};
};

}  // namespace gc::hazard
