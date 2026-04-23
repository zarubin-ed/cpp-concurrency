#pragma once

#include <exe/future/type/future.hpp>

#include <memory>
#include <cstdlib>  // std::abort

namespace exe::future {

template <typename T>
Future<T> First(Future<T> f1, Future<T> f2) {
  auto [future, promise] = Contract<T>();

  struct ProtectedPromise {
    twist::ed::std::atomic<bool> is_satisfied;
    Promise<T> promice;
  };

  std::shared_ptr<ProtectedPromise> state =
      std::make_shared<ProtectedPromise>(false, std::move(promise));

  auto cb = [state](T value) {
    if (!state->is_satisfied.exchange(true)) {
      std::move(state->promice).Set(std::move(value));
    }
  };

  std::move(f1).Consume(cb);
  std::move(f2).Consume(cb);

  return std::move(future);
}

}  // namespace exe::future
