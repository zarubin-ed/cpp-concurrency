#pragma once

#include <exe/future/type/result.hpp>
#include <exe/future/make/ready.hpp>
#include <exe/result/make/ok.hpp>

namespace exe::future {

template <typename T, typename E>
TryFuture<T, E> Ok(T value) {
  return Ready(result::Ok<T, E>(std::move(value)));
}

}  // namespace exe::future
