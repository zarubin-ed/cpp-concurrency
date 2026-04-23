#pragma once

#include <exe/result/type/result.hpp>

namespace exe::result::trait {

namespace match {

template <typename T>
struct IsResult {
  static constexpr bool kValue = false;
};

template <typename T, typename E>
struct IsResult<Result<T, E>> {
  static constexpr bool kValue = true;
};

}  // namespace match

template <typename T>
static constexpr bool IsResult = match::IsResult<T>::kValue;  // NOLINT

}  // namespace exe::result::trait
