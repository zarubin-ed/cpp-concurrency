#pragma once

#include <exe/future/make/contract.hpp>
#include <exe/future/syntax/pipe.hpp>

#include <exe/runtime/set_timer.hpp>

#include <chrono>

namespace exe::future {

namespace pipe {

struct [[nodiscard]] After {
  std::chrono::microseconds delay;

  explicit After(std::chrono::microseconds d)
      : delay(d) {
  }

  // Non-copyable
  After(const After&) = delete;

  template <typename T>
  Future<T> Pipe(Future<T> value) {
    auto [future, promise] = Contract<T>();
    auto rt = value.GetRuntime();
    future.SetRuntime(rt);

    std::move(value).Consume(
        [delay = delay, rt = rt, promise = std::move(promise)](T v) mutable {
          runtime::SetTimer(
              rt, delay,
              [promise = std::move(promise), v = std::move(v)]() mutable {
                std::move(promise).Set(std::move(v));
              });
        });

    return std::move(future);
  }
};

}  // namespace pipe

/*
 * Delays asynchronous computation
 *
 * Future<T> -> std::chrono::microseconds -> Future<T>
 *
 */

inline auto After(std::chrono::microseconds delay) {
  return pipe::After{delay};
}

}  // namespace exe::future
