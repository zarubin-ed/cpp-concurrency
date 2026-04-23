#pragma once

#include <exe/result/type/result.hpp>
#include <exe/result/trait/value_of.hpp>
#include <exe/result/syntax/pipe.hpp>

#include <type_traits>  // std::invoke_result_t

namespace exe::result {

namespace pipe {

template <typename F>
struct [[nodiscard]] AndThen {
  F user;

  explicit AndThen(F u)
      : user(std::move(u)) {
  }

  // Non-copyable
  AndThen(const AndThen&) = delete;
  AndThen& operator=(const AndThen&) = delete;

  template <typename T>
  using U = result::trait::ValueOf<std::invoke_result_t<F, T>>;

  template <typename T, typename E>
  Result<U<T>, E> Pipe(Result<T, E> r) {
    return std::move(r).and_then(user);
  }
};

}  // namespace pipe

/*
 * Result<T, E> -> (T -> Result<U, E>) -> Result<U, E>
 *
 * Usage:
 *
 * auto r = result::Ok<int, Unit>(1) | result::AndThen([](int v) {
 *   return result::Ok(v + 1);
 * });
 *
 */

template <typename F>
auto AndThen(F user) {
  return pipe::AndThen{std::move(user)};
}

}  // namespace exe::result
