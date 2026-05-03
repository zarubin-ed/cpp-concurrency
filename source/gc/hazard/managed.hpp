#pragma once

#include <concepts>
#include <vvv/list.hpp>

namespace gc::hazard {

struct Deleter : vvv::IntrusiveListNode<Deleter> {
  void (*destroy)(Deleter*) = nullptr;

  void Destroy() {
    destroy(this);
  }
};

template <typename T>
struct Managed : Deleter {
  Managed() {
    destroy = &Managed::DestroyImpl;
  }

 private:
  static void DestroyImpl(Deleter* obj) {
    delete static_cast<T*>(obj);
  }
};

template <typename T>
concept IsManaged = std::derived_from<T, Managed<T>>;

}  // namespace gc::hazard
