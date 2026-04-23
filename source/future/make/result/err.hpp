#pragma once

#include <exe/future/type/result.hpp>

#include <exe/future/make/ready.hpp>
#include <exe/result/make/err.hpp>

namespace exe::future {

/*
 * Usage:
 *
 * future::TryFuture<int, std::error_code> TimedOut() {
 *   return future::Err<int, std::error_code>(TimeoutError());
 * }
 *
 */

template <typename T, typename E>
TryFuture<T, E> Err(E error) {
  return future::Ready(result::Err<T, E>(std::move(error)));
}

}  // namespace exe::future
