#pragma once

#include <cstddef>
#include <utility>

namespace exe::future {

namespace detail {

template <typename T>
class ManualStorage {
 public:
  template <typename... Args>
  void Emplace(Args&&... args) {
    new (data_) T(std::forward<Args>(args)...);
  }

  ~ManualStorage() {
    operator->()->~T();
  }

  T& operator*() {
    return *operator->();
  }

  T* operator->() {
    return reinterpret_cast<T*>(data_);
  }

 private:
  alignas(alignof(T)) std::byte data_[sizeof(T)];
};
}  // namespace detail

}  // namespace exe::future