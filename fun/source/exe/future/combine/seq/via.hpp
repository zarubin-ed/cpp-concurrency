#pragma once
#include <exe/future/make/contract.hpp>

#include <exe/runtime/view.hpp>
#include <exe/runtime/submit_task.hpp>

#include <exe/future/syntax/pipe.hpp>

#include <cstdlib>  // std::abort

namespace exe::future {

namespace pipe {

struct [[nodiscard]] Via {
  runtime::View runtime;

  explicit Via(runtime::View rt)
      : runtime(rt) {
  }

  // Non-copyable
  Via(const Via&) = delete;

  template <typename T>
  Future<T> Pipe(Future<T> future) {
    future.SetRuntime(runtime);
    return std::move(future);
  }
};

}  // namespace pipe

/*
 * Set runtime
 *
 * Future<T> -> Runtime -> Future<T>
 *
 */

inline auto Via(runtime::View runtime) {
  return pipe::Via{runtime};
}

}  // namespace exe::future
