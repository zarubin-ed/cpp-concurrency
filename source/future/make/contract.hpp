#pragma once

#include <exe/future/type/future.hpp>

#include <exe/future/detail/contract_state.hpp>

#include <cassert>
#include <tuple>

namespace exe::future {

// Producer

template <typename T>
class Promise {
  using State = detail::ContractState<T>;

 public:
  template <typename U>
  friend std::tuple<Future<U>, Promise<U>> Contract();

  // Move-constructible
  Promise(Promise&& promise)
      : cs_(promise.cs_) {
    promise.ReleaseState();
  }

  // Non-copyable
  Promise(const Promise&) = delete;
  Promise& operator=(const Promise&) = delete;

  // Non-move-assignable
  Promise& operator=(Promise&&) = delete;

  ~Promise() {
    assert(IsSatisfied());
  }

  // One-shot
  template <typename U>
  void Set(U&& value) && {
    ReleaseState()->Produce(std::forward<U>(value));
  }

  // One-shot
  void Set(T value) && {
    ReleaseState()->Produce(std::move(value));
  }

  bool IsSatisfied() {
    return cs_ == nullptr;
  }

 private:
  State* ReleaseState() {
    return std::exchange(cs_, nullptr);
  }

  explicit Promise(State* cs)
      : cs_(cs) {
  }

  State* cs_;
};

/*
 * Asynchronous one-shot contract
 *
 * Usage:
 *
 * auto [f, p] = future::Contract<int>();
 *
 * std::thread producer([p = std::move(p)] mutable {  // clippy-no-censor
 *   // Producer
 *   std::move(p).Set(7);
 * });
 *
 * // Consumer
 * auto v = future::Get(std::move(f));  // 7
 *
 * producer.join();
 *
 */

template <typename T>
std::tuple<Future<T>, Promise<T>> Contract() {
  auto cs = new detail::ContractState<T>();

  Promise<T> p(cs);
  Future<T> f(cs);

  return std::move(std::tuple(std::move(f), std::move(p)));
}

}  // namespace exe::future
