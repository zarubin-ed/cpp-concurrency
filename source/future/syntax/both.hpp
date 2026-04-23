#pragma once

#include <exe/future/combine/concur/all.hpp>

#include <utility>  // std::move

/*
 * Syntactic sugar for future::Both combinator
 *
 * Future<A> * Future<B> -> Future<std::tuple<A, B>>
 * https://en.wikipedia.org/wiki/Product_type
 *
 * Usage:
 *
 * auto both = std::move(lhs) * std::move(rhs);
 *
 */

template <typename A, typename B>
auto operator*(exe::future::Future<A> lhs, exe::future::Future<B> rhs) {
  return exe::future::Both(std::move(lhs), std::move(rhs));
}
