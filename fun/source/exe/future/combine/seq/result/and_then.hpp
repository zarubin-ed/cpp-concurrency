#pragma once

#include <exe/future/type/result.hpp>

#include <exe/result/trait/value_of.hpp>
#include <exe/future/trait/value_of.hpp>

#include <exe/runtime/submit_task.hpp>

#include <type_traits>
#include <utility>  // std::move

namespace exe::future {

namespace pipe {

template <typename F>
struct [[nodiscard]] AndThen {
  F user;

  explicit AndThen(F u)
      : user(std::move(u)) {
  }

  // Non-copyable
  AndThen(const AndThen&) = delete;

  template <typename T>
  using U = result::trait::ValueOf<
      future::trait::ValueOf<std::invoke_result_t<F, T>>>;

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

      runtime::SubmitTask(
          rt, [promise = std::move(promise), user = std::move(user),
               value = std::move(exp.value())]() mutable {
            auto result = user(std::move(value));

            std::move(result).Consume(
                [promise = std::move(promise)](Try<U<T>, E> exp) mutable {
                  std::move(promise).Set(std::move(exp));
                });
          });
    });

    return std::move(future);
  }
};

}  // namespace pipe

/*
 * Asynchronous try-catch / happy path
 *
 * TryFuture<T, E> -> (T -> TryFuture<U, E>) -> TryFuture<U, E>
 *
 */

template <typename F>
auto AndThen(F user) {
  return pipe::AndThen{std::move(user)};
}

}  // namespace exe::future
