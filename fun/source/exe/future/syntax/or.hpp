#pragma once

#include <exe/future/combine/concur/first.hpp>

#include <utility>  // std::move

/*
 * Syntactic sugar for future::First combinator
 *
 * Usage:
 *
 * auto first = std::move(lhs) or std::move(rhs);
 *
 */

template <typename T>
exe::future::Future<T> operator||(exe::future::Future<T> lhs,
                                  exe::future::Future<T> rhs) {
  return exe::future::First(std::move(lhs), std::move(rhs));
}
