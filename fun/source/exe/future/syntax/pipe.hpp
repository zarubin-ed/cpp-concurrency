#pragma once

#include <exe/future/type/future.hpp>

#include <utility>  // std::move

/*
 * Pipeline operator for sequential Future composition
 *
 * Usage:
 *
 * auto f = future::Value(7) | future::Map([](int v) { return v + 1; });
 *
 */

template <typename T, typename Combinator>
auto operator|(exe::future::Future<T> f, Combinator c) {
  return c.Pipe(std::move(f));
}
