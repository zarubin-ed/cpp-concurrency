#pragma once

#include "contract.hpp"

#include <cstdlib>  // std::abort

namespace exe::future {

/*
 * Ready value
 *
 * Usage:
 *
 * auto f = future::Ready(result::Ok<int, ExampleError>(42));
 *
 */

template <typename T>
Future<T> Ready(T value) {
  auto [future, promise] = Contract<T>();

  std::move(promise).Set(std::move(value));

  return std::move(future);
}

}  // namespace exe::future
