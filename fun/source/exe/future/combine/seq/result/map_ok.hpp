#pragma once

#include <exe/future/type/result.hpp>

#include <exe/runtime/submit_task.hpp>

#include <type_traits>
#include <utility>  // std::move

namespace exe::future {

namespace pipe {

template <typename F>
struct [[nodiscard]] MapOk {
  F user;

  explicit MapOk(F u)
      : user(std::move(u)) {
  }

  // Non-copyable
  MapOk(const MapOk&) = delete;

  template <typename T>
  using U = std::invoke_result_t<F, T>;

  template <typename T, typename E>
  TryFuture<U<T>, E> Pipe(TryFuture<T, E> tf) {
    auto [future, promise] = Contract<Try<U<T>, E>>();
    auto rt = tf.GetRuntime();
    future.SetRuntime(rt);

    std::move(tf).Consume([rt = rt, promise = std::move(promise),
                           user = std::move(user)](Try<T, E> exp) mutable {
      if (!exp) {
        std::move(promise).Set(
            Try<U<T>, E>(std::unexpected(std::move(exp.error()))));
        return;
      }

      runtime::SubmitTask(rt,
                          [promise = std::move(promise), user = std::move(user),
                           value = std::move(exp.value())]() mutable {
                            std::move(promise).Set(user(std::move(value)));
                          });
    });

    return std::move(future);
  }
};

}  // namespace pipe

// TryFuture<T, E> -> (T -> U) -> TryFuture<U, E>

template <typename F>
auto MapOk(F user) {
  return pipe::MapOk(std::move(user));
}

}  // namespace exe::future
