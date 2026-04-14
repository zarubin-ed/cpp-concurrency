#pragma once

#include <exe/future/syntax/pipe.hpp>

#include <utility>  // std::move

namespace exe::future {

/*
 * Consumes Future, ignores value
 *
 * Considered harmful
 *
 */

template <typename T>
void Detach(Future<T> value) {
  std::move(value).Consume([](T) {});
}

// Chaining

namespace pipe {

struct [[nodiscard]] Detach {
  template <typename T>
  void Pipe(Future<T> f) {
    future::Detach(std::move(f));
  }
};

}  // namespace pipe

inline auto Detach() {
  return pipe::Detach{};
}

}  // namespace exe::future
