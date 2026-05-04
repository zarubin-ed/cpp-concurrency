#pragma once

#include <gc/hazard/user.hpp>

#include <twist/ed/std/atomic.hpp>

#include <cassert>
#include <new>
#include <memory>

namespace gc {

//////////////////////////////////////////////////////////////////////
namespace detail {

template <typename T>
struct SharedState : hazard::Managed<SharedState<T>> {
  twist::ed::std::atomic<uint64_t> counter_{1};
  alignas(alignof(T)) std::byte data_[sizeof(T)];

  template <typename... Args>
  explicit SharedState(Args&&... args) {
    std::construct_at(Ptr(), std::forward<Args>(args)...);
  }

  ~SharedState() {
    std::destroy_at(Ptr());
  }

  T* Ptr() {
    return reinterpret_cast<T*>(data_);
  }

  void Unlink() {
    if (counter_.fetch_sub(1) == 1) {
      gc::hazard::GetMutator()->Retire(this);
    }
  }

  void AddRef() {
    counter_.fetch_add(1);
  }

  bool TryAddRef() {
    uint64_t old = counter_.load();
    while (old != 0) {
      if (counter_.compare_exchange_weak(old, old + 1)) {
        return true;
      }
    }
    return false;
  }
};

}  // namespace detail

//////////////////////////////////////////////////////////////////////

template <typename T>
class SharedPtr {
  template <typename U, typename... Args>
  friend SharedPtr<U> MakeShared(Args&&...);

  template <typename>
  friend class AtomicSharedPtr;

  explicit SharedPtr(detail::SharedState<T>* ss)
      : ss_(ss) {
  }

 public:
  SharedPtr() = default;

  SharedPtr(const SharedPtr<T>& that)
      : ss_(that.ss_) {
    Init();
  }

  SharedPtr<T>& operator=(const SharedPtr<T>& that) {
    SharedPtr temp(that);
    Swap(temp);
    return *this;
  }

  SharedPtr(SharedPtr<T>&& that) noexcept
      : ss_(that.ss_) {
    that.ss_ = nullptr;
  }

  SharedPtr<T>& operator=(SharedPtr<T>&& that) {
    SharedPtr temp(std::move(that));
    Swap(temp);
    return *this;
  }

  void Swap(SharedPtr<T>& that) {
    std::swap(that.ss_, ss_);
  }

  T* operator->() const {
    return ss_->Ptr();
  }

  T& operator*() const {
    return *operator->();
  }

  explicit operator bool() const {
    return ss_ != nullptr;
  }

  void Reset() {
    if (!operator bool()) {
      return;
    }

    std::exchange(ss_, nullptr)->Unlink();
  }

  ~SharedPtr() {
    Reset();
  }

 private:
  void Init() {
    if (operator bool()) {
      ss_->AddRef();
    }
  }

  mutable detail::SharedState<T>* ss_ = nullptr;
};

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
  auto ss = new detail::SharedState<T>(std::forward<Args>(args)...);
  return SharedPtr(ss);
}

//////////////////////////////////////////////////////////////////////

template <typename T>
class AtomicSharedPtr {
  using State = detail::SharedState<T>;

 public:
  AtomicSharedPtr() = default;

  ~AtomicSharedPtr() {
    Store(SharedPtr<T>{});
  }

  SharedPtr<T> Load() {
    hazard::Mutator* mutator = gc::hazard::GetMutator();
    hazard::PtrOwner protected_ss = mutator->GetHazardPtr(0);

    while (true) {
      auto sr = protected_ss.ScopedReset();

      State* ss = protected_ss.Protect(ss_);

      if (ss == nullptr) {
        return {};
      }

      if (ss->TryAddRef()) {
        return SharedPtr<T>(ss);
      }
    }
  }

  void Store(SharedPtr<T> target) {
    State* new_ss = target.ss_;

    if (new_ss != nullptr) {
      new_ss->AddRef();
    }

    State* old_ss = ss_.exchange(new_ss);

    if (old_ss != nullptr) {
      old_ss->Unlink();
    }
  }

  explicit operator SharedPtr<T>() {
    return Load();
  }

  bool CompareExchangeWeak(SharedPtr<T>& expected, SharedPtr<T> desired) {
    State* ss_desired = desired.ss_;
    State* ss_expected = expected.ss_;

    if (ss_desired != nullptr) {
      ss_desired->AddRef();
    }

    hazard::Mutator* mutator = gc::hazard::GetMutator();
    hazard::PtrOwner protected_ss = mutator->GetHazardPtr(0);
    while (true) {
      auto sr = protected_ss.ScopedReset();

      State* ss = protected_ss.Protect(ss_);
      if (ss_expected == ss) {
        if (ss_.compare_exchange_weak(ss, ss_desired)) {
          if (ss != nullptr) {
            ss->Unlink();
          }
          return true;
        }
      } else {
        if (ss == nullptr) {
          if (ss_desired != nullptr) {
            ss_desired->Unlink();
          }
          expected = SharedPtr<T>{};
          return false;
        }

        if (!ss->TryAddRef()) {
          continue;
        }

        if (ss_desired != nullptr) {
          ss_desired->Unlink();
        }

        expected = SharedPtr<T>(ss);
        return false;
      }
    }
  }

 private:
  mutable twist::ed::std::atomic<detail::SharedState<T>*> ss_ = nullptr;
};

}  // namespace gc
