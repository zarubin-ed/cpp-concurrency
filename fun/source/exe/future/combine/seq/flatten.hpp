#pragma once

#include <exe/future/syntax/pipe.hpp>

#include <cstdlib>  // std::abort

namespace exe::future {

namespace pipe {

struct [[nodiscard]] Flatten {
  Flatten() = default;

  // Non-copyable
  Flatten(const Flatten&) = delete;

  template <typename T>
  Future<T> Pipe(Future<Future<T>> value) {
    auto [future, promise] = Contract<T>();
    future.SetRuntime(value.GetRuntime());

    std::move(value).Consume([promise =
                                  std::move(promise)](Future<T> value) mutable {
      std::move(value).Consume([promise = std::move(promise)](T value) mutable {
        std::move(promise).Set(std::move(value));
      });
    });

    return std::move(future);
  }
};

}  // namespace pipe

/*
 * Collapses nested Future-s
 *
 * Monadic join
 * https://wiki.haskell.org/Typeclassopedia
 *
 * LTL: ♢♢phi = ♢phi
 * https://lamport.azurewebsites.net/tla/science.pdf
 *
 * Future<Future<T>> -> Future<T>
 *
 */

inline auto Flatten() {
  return pipe::Flatten{};
}

}  // namespace exe::future
