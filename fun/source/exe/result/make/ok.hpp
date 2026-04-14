#pragma once

#include <exe/result/type/result.hpp>

namespace exe::result {

template <typename T, typename E>
Result<T, E> Ok(T value) {
  return {std::move(value)};
}

}  // namespace exe::result
