#pragma once

#include <exe/future/type/result.hpp>

#include <exe/runtime/submit_task.hpp>

#include <utility>  // std::move

namespace exe::future {

namespace pipe {

template <typename F>
struct [[nodiscard]] OrElse {
  F user;

  explicit OrElse(F u)
      : user(std::move(u)) {
  }

  // Non-copyable
  OrElse(const OrElse&) = delete;

  template <typename T, typename E>
  TryFuture<T, E> Pipe(TryFuture<T, E> tf) {
    auto [future, promise] = Contract<Try<T, E>>();
    auto rt = tf.GetRuntime();
    future.SetRuntime(rt);

    std::move(tf).Consume([rt = rt, promise = std::move(promise),
                           user = std::move(user)](Try<T, E> exp) mutable {
      if (exp) {
        std::move(promise).Set(std::move(std::move(exp.value())));
        return;
      }

      runtime::SubmitTask(
          rt, [promise = std::move(promise), user = std::move(user),
               error = exp.error()]() mutable {
            auto result = user(std::move(error));

            std::move(result).Consume(
                [promise = std::move(promise)](Try<T, E> exp) mutable {
                  std::move(promise).Set(std::move(exp));
                });
          });
    });

    return std::move(future);
  }
};

}  // namespace pipe

/*
 * Asynchronous catch
 *
 * TryFuture<T, E> -> (E -> TryFuture<T, E>) -> TryFuture<T, E>
 *
 */

template <typename F>
auto OrElse(F user) {
  return pipe::OrElse{std::move(user)};
}

}  // namespace exe::future
