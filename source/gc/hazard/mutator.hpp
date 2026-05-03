#pragma once

#include "fwd.hpp"
#include "limits.hpp"
#include "managed.hpp"
#include "owner.hpp"

#include <array>
#include <cstdlib>

namespace gc::hazard {

class Mutator {
  friend class Collector;

 public:
  explicit Mutator(Collector* collector)
      : collector_(collector) {
  }

  PtrOwner GetHazardPtr(size_t index) {
    return PtrOwner{&(hazard_ptrs_[index])};
  }

  template <typename T>
  void Retire(T* obj) {
    static_assert(IsManaged<T>);

    retired_.PushBack(static_cast<Deleter*>(obj));
    Gc();
  }

  void Gc();

 private:
  [[maybe_unused]] Collector* collector_;

  std::array<PtrRecord, kNumHazardPtrsPerThread> hazard_ptrs_;

  vvv::IntrusiveList<Deleter> retired_;

  twist::ed::std::atomic<Mutator*> next_{
      nullptr};  // for intrusive list in collector
};

}  // namespace gc::hazard
