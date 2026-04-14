#pragma once

#include <exe/future/syntax/pipe.hpp>

#include <exe/future/detail/manual_storage.hpp>

#include <exe/thread/event.hpp>

#include <utility>  // std::move

namespace exe::future {

/*
 * Unwraps Future synchronously (blocking current thread)
 *
 * Usage:
 *
 * auto f = future::Spawn(runtime, [] { return 7; }));
 * auto v = future::Get(std::move(f));
 *
 */

template <typename T>
T Get(Future<T> future) {
  thread::Event event;
  detail::ManualStorage<T> result;

  std::move(future).Consume([&](T value) {
    result.Emplace(std::move(value));
    event.Fire();
  });

  event.Wait();
  return std::move(*result);
}

// Chaining

namespace pipe {

struct [[nodiscard]] Get {
  template <typename T>
  T Pipe(Future<T> f) {
    return future::Get(std::move(f));
  }
};

}  // namespace pipe

/*
 * Usage:
 *
 * auto v = future::Spawn(runtime, [] { return 7; }) | future::Get();
 */

inline auto Get() {
  return pipe::Get{};
}

}  // namespace exe::future
