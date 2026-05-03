#pragma once

#include "record.hpp"
#include "reset_guard.hpp"
#include "managed.hpp"

#include <twist/trace/scope.hpp>

namespace gc::hazard {

class PtrOwner {
  friend class Mutator;

 public:
  // Non-copyable
  PtrOwner(const PtrOwner&) = delete;
  PtrOwner& operator=(const PtrOwner&) = delete;

  template <typename T>
  T* Protect(twist::ed::std::atomic<T*>& atomic_ptr,
             twist::trace::Scope = twist::trace::Scope(owner, "Protect")) {
    static_assert(IsManaged<T>);

    while (true) {
      T* ptr = atomic_ptr.load();
      Set(ptr);
      if (atomic_ptr.load() == ptr) {
        return ptr;
      }
    }
  }

  template <typename T>
  void Set(T* obj, twist::trace::Scope = twist::trace::Scope(owner, "Set")) {
    static_assert(IsManaged<T>);

    hazard_ptr_->Set(obj);
  }

  void Reset(twist::trace::Scope = twist::trace::Scope(owner, "Reset")) {
    hazard_ptr_->Reset();
  }

  auto ScopedReset() {
    return PtrResetGuard{*this};
  }

  ~PtrOwner() {
    Reset();
    Release();
  }

 private:
  explicit PtrOwner(PtrRecord* hp)
      : hazard_ptr_(hp) {
  }

  void Release() {
    // No need for now
  }

 private:
  // Tracing
  static inline twist::trace::Domain owner{"HazardPtrOwner"};

 private:
  [[maybe_unused]] PtrRecord* hazard_ptr_;
  // ???
};

}  // namespace gc::hazard
