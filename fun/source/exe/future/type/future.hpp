#pragma once

#include <exe/future/detail/callback.hpp>
#include <exe/future/detail/contract_state.hpp>

namespace exe::future {

// Represents future value of type T

template <typename>
class Promise;

template <typename T>
class [[nodiscard]] Future {
  using State = detail::ContractState<T>;

 public:
  template <typename U>
  friend std::tuple<Future<U>, Promise<U>> Contract();

  using ValueType = T;

  // Future

  // Move-constructible
  Future(Future&& future)
      : cs_(future.cs_) {
    future.cs_ = nullptr;
  }

  // Non-copyable
  Future(const Future&) = delete;
  Future& operator=(const Future&) = delete;

  // Non-move-assignable
  Future& operator=(Future&&) = delete;

  ~Future() {
    assert(IsConsumed());
  }

  // One-shot
  void Consume(detail::Callback<T> cb) && {
    ReleaseState()->Consume(std::move(cb));
  }

  bool IsConsumed() {
    return cs_ == nullptr;
  }

  void SetRuntime(runtime::View runtime) {
    cs_->SetRuntime(runtime);
  }

  runtime::View GetRuntime() {
    return cs_->GetRuntime();
  }

 private:
  State* ReleaseState() {
    return std::exchange(cs_, nullptr);
  }

  explicit Future(State* cs)
      : cs_(cs) {
  }

  State* cs_;
};

}  // namespace exe::future
