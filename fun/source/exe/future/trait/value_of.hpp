#pragma once

#include <exe/future/type/future.hpp>

namespace exe::future {

namespace trait {

namespace match {

template <typename T>
struct ValueOf;

template <typename T>
struct ValueOf<Future<T>> {
  using Type = T;
};

}  // namespace match

template <typename Future>
using ValueOf = typename match::ValueOf<Future>::Type;

}  // namespace trait

}  // namespace exe::future
