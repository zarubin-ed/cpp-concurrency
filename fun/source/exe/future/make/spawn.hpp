#pragma once

#include "contract.hpp"

#include <exe/future/type/future.hpp>

#include <exe/runtime/view.hpp>
#include <exe/runtime/submit_task.hpp>

#include <type_traits>

namespace exe::future {

/*
 * Computation (to be) scheduled to the given runtime
 *
 * Usage:
 *
 * auto f = future::Spawn(runtime, [] {
 *   return 42;  // ~ computation
 * });
 *
 */

template <typename F>
Future<std::invoke_result_t<F>> Spawn(runtime::View runtime, F user) {
  auto [future, promise] = Contract<std::invoke_result_t<F>>();
  future.SetRuntime(runtime);

  runtime::SubmitTask(runtime, [promise = std::move(promise),
                                user = std::move(user)]() mutable {
    std::move(promise).Set(user());
  });

  return std::move(future);
}

}  // namespace exe::future
