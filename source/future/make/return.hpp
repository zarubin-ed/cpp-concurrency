#pragma once

#include "ready.hpp"

#include <utility>  // std::move

namespace exe::future {

/*
 * Monadic return
 * https://wiki.haskell.org/Typeclassopedia
 *
 * Synonym for Ready
 *
 * Usage:
 *
 * auto f = future::Return(11);
 *
 */

template <typename T>
Future<T> Return(T value) {
  return Ready(std::move(value));
}

}  // namespace exe::future
