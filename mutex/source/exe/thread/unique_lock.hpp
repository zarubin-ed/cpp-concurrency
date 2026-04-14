
namespace exe::thread {

template <typename T>
class UniqueLock {
 public:
  explicit UniqueLock(T& mutex)
      : mutex_(&mutex) {
    lock();
  }

  void lock() {  // NOLINT
    if (!is_owned_) {
      mutex_->lock();
    }
    is_owned_ = true;
  }

  void unlock() {  // NOLINT
    if (is_owned_) {
      is_owned_ = false;
      mutex_->unlock();
    }
  }

  ~UniqueLock() {
    if (is_owned_) {
      mutex_->unlock();
    }
  }

 private:
  T* mutex_ = nullptr;
  bool is_owned_ = false;
};

}  // namespace exe::thread