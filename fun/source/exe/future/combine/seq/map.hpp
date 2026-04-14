#pragma once

#include <exe/future/syntax/pipe.hpp>
#include <exe/runtime/submit_task.hpp>

#include <type_traits>
#include <utility>  // std::move

namespace exe::future {

namespace pipe {

template <typename F>
struct [[nodiscard]] Map {
  F user;

  explicit Map(F u)
      : user(std::move(u)) {
  }

  // Non-copyable
  Map(const Map&) = delete;

  template <typename T>
  using U = std::invoke_result_t<F, T>;

  template <typename T>
  Future<U<T>> Pipe(Future<T> value) {
    auto [future, promise] = Contract<U<T>>();
    auto rt = value.GetRuntime();
    future.SetRuntime(rt);

    std::move(value).Consume([rt = rt, promise = std::move(promise),
                              user = std::move(user)](T value) mutable {
      exe::runtime::SubmitTask(
          rt, [promise = std::move(promise), user = std::move(user),
               value = std::move(value)]() mutable {
            auto out = user(std::move(value));
            std::move(promise).Set(std::move(out));
          });
    });

    return std::move(future);
  }
};

}  // namespace pipe

/*
 * Functor
 * https://wiki.haskell.org/Typeclassopedia
 *
 * Future<T> -> (T -> U) -> Future<U>
 *
 */

template <typename F>
auto Map(F user) {
  return pipe::Map{std::move(user)};
}

}  // namespace exe::future
