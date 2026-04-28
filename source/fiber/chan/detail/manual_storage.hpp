#include <cstddef>
#include <memory>
#include <utility>

namespace exe::fiber {

namespace detail {

template <typename T>
class ManualStorage {
 public:
  template <typename... Args>
  void Emplace(Args&&... args) {
    std::construct_at(operator->(), std::forward<Args>(args)...);
  }

  ~ManualStorage() {
    std::destroy_at(operator->());
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

}  // namespace exe::fiber
