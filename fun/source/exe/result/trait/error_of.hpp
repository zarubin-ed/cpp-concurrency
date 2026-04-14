#pragma once

#include <exe/result/type/result.hpp>

namespace exe::result::trait {

namespace match {

template <typename T>
struct ErrorOf;

template <typename T, typename E>
struct ErrorOf<Result<T, E>> {
  using Type = E;
};

}  // namespace match

template <typename Result>
using ErrorOf = typename match::ErrorOf<Result>::Type;

}  // namespace exe::result::trait
