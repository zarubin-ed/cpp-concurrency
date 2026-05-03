#pragma once

namespace gc::hazard {

class PtrOwner;

class PtrResetGuard {
 public:
  explicit PtrResetGuard(PtrOwner& owner)
      : owner_(owner) {
  }

  // Non-copyable
  PtrResetGuard(const PtrResetGuard&) = delete;
  PtrResetGuard& operator=(const PtrResetGuard) = delete;

  ~PtrResetGuard();

 private:
  PtrOwner& owner_;
};

}  // namespace gc::hazard
