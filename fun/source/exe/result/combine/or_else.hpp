#pragma once

#include <exe/result/type/result.hpp>
#include <exe/result/syntax/pipe.hpp>

namespace exe::result {

namespace pipe {

template <typename F>
struct [[nodiscard]] OrElse {
  F user;

  explicit OrElse(F u)
      : user(std::move(u)) {
  }

  // Non-copyable
  OrElse(const OrElse&) = delete;
  OrElse& operator=(const OrElse&) = delete;

  template <typename T, typename E>
  Result<T, E> Pipe(Result<T, E> r) {
    return std::move(r).or_else(user);
  }
};

}  // namespace pipe

/*
 * Result<T, E> -> (E -> Result<T, E>) -> Result<T, E>
 *
 * Usage:
 *
 * auto r = result::Err<int, ExampleError>({})
 *            | result::OrElse([](ExampleError) {
 *                return result::Ok<int, ExampleError>(42);  // Fallback
 *              });
 *
 */

template <typename F>
auto OrElse(F user) {
  return pipe::OrElse{std::move(user)};
}

}  // namespace exe::result
