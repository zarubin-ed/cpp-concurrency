#include "collector.hpp"

#include <twist/ed/static/var.hpp>
#include <twist/ed/static/thread_local/ptr.hpp>

namespace gc::hazard {

//////////////////////////////////////////////////////////////////////

PtrResetGuard::~PtrResetGuard() {
  owner_.Reset();
}

//////////////////////////////////////////////////////////////////////

Collector& Collector::Instance() {
  TWISTED_STATIC(Collector, instance);
  return *instance;
}

Collector::~Collector() {  // head_ is invalid
  // collect garbage
  auto* cur = head_.load();
  while (cur != nullptr) {
    cur->Gc();
    cur = cur->next_.load();
  }

  // delete mutators
  cur = head_.load();
  while (cur != nullptr) {
    auto* tmp = cur->next_.load();
    delete cur;
    cur = tmp;
  }
}

TWISTED_STATIC_THREAD_LOCAL_PTR(Mutator, current_mutator);

Mutator* Collector::GetMutator() {
  if (current_mutator == nullptr) {
    current_mutator = new Mutator(this);

    while (true) {
      auto* head = head_.load();
      current_mutator->next_.store(head);
      if (head_.compare_exchange_weak(head, current_mutator)) {
        break;
      }
    }
  }
  return current_mutator;
}

//////////////////////////////////////////////////////////////////////

Mutator* GetMutator() {
  return Collector::Instance().GetMutator();
}

}  // namespace gc::hazard
