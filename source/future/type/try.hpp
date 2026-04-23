#pragma once

#include "future.hpp"

#include <exe/result/type/try.hpp>

namespace exe::future {

// Fallible Future

template <typename T, typename E>
using TryFuture = Future<Try<T, E>>;

}  // namespace exe::future
