#pragma once

#include "spawn.hpp"

namespace exe::future {

/*
 * Computation (to be) scheduled to the given runtime
 *
 * Synonym for Spawn
 *
 * Usage:
 *
 * auto f = future::Run(runtime, [] {
 *   return 42;  // ~ computation
 * });
 *
 */

template <typename F>
Future<std::invoke_result_t<F>> Run(runtime::View runtime, F user) {
  return Spawn(runtime, std::move(user));
}

}  // namespace exe::future
