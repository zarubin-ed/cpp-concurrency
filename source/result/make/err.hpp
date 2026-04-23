#pragma once

#include <exe/result/type/result.hpp>

namespace exe::result {

template <typename T, typename E>
Result<T, E> Err(E error) {
  return std::unexpected(std::move(error));
}

}  // namespace exe::result
