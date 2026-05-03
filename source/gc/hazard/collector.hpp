#pragma once

#include "mutator.hpp"

namespace gc::hazard {

class Collector {
  friend class Mutator;

 public:
  static Collector& Instance();

  ~Collector();

  Mutator* GetMutator();

  bool IsProtected(void* obj) const {
    while (true) {
      auto* head = head_.load();
      auto* cur = head;

      while (cur != nullptr) {
        for (auto& hp : cur->hazard_ptrs_) {
          if (hp.Get() == obj) {
            return true;
          }
        }
        cur = cur->next_.load();
      }

      if (head_.load() == head) {
        break;
      }
    }
    return false;
  }

 private:
  twist::ed::std::atomic<Mutator*> head_{nullptr};
};

}  // namespace gc::hazard
