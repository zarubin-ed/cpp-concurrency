#include <memory>
#include <cassert>
#include <cstdlib>

namespace exe::fiber {
namespace detail {
template <typename T>
class CircularBuffer {
 public:
  CircularBuffer(const CircularBuffer&) = delete;
  CircularBuffer& operator=(const CircularBuffer&) = delete;

  explicit CircularBuffer(size_t capacity)
      : capacity_(capacity + 1),
        data_(new std::byte[(capacity_) * sizeof(T)]) {
  }

  template <typename... Args>
  void Emplace(Args&&... args) {
    assert(!IsFull());

    size_t pos = PostIncrement(tail_);
    std::construct_at(&operator[](pos), std::forward<Args>(args)...);
  }

  T Pop() {
    assert(!IsEmpty());

    size_t pos = PostIncrement(head_);
    T ret = std::move(operator[](pos));

    std::destroy_at(&operator[](pos));
    return ret;
  }

  const T& operator[](size_t idx) const {
    return Data()[idx];
  }

  T& operator[](size_t idx) {
    return Data()[idx];
  }

  T* Data() {
    return reinterpret_cast<T*>(data_);
  }

  size_t PreIncrement(size_t& val) {
    val = (val + 1) % capacity_;
    return val;
  }

  size_t PostIncrement(size_t& val) {
    return std::exchange(val, (val + 1) % capacity_);
  }

  size_t Size() const {
    if (tail_ < head_) {
      return tail_ + capacity_ - head_;
    }
    return tail_ - head_;
  }

  size_t Capacity() const {
    return capacity_ - 1;
  }

  bool IsFull() const {
    return Size() == Capacity();
  }

  bool IsEmpty() const {
    return Size() == 0;
  }

  ~CircularBuffer() {
    for (size_t i = head_; i != tail_; i = PreIncrement(i)) {
      std::destroy_at(&operator[](i));
    }
    delete[] data_;
  }

 private:
  size_t capacity_ = 0;
  size_t tail_ = 0;
  size_t head_ = 0;

  std::byte* data_;
};
}  // namespace detail

}  // namespace exe::fiber