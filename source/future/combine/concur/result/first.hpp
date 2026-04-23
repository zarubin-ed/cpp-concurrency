#pragma once

#include <exe/future/type/result.hpp>

namespace exe::future {

// First success / last error

template <typename T, typename E>
TryFuture<T, E> FirstOk(TryFuture<T, E> f1, TryFuture<T, E> f2) {
  auto [future, promise] = Contract<Try<T, E>>();

  struct ProtectedPromise {
    Promise<Try<T, E>> promise;
    enum : int { Satisfied = 1, ReportError = 2, ReturnValue = 0 };

    explicit ProtectedPromise(Promise<Try<T, E>> p)
        : promise(std::move(p)) {
    }

    twist::ed::std::atomic_int state{ReturnValue};
  };

  std::shared_ptr<ProtectedPromise> state =
      std::make_shared<ProtectedPromise>(std::move(promise));

  auto cb = [state](Try<T, E> exp) {
    if (exp && state->state.exchange(ProtectedPromise::Satisfied) !=
                   ProtectedPromise::Satisfied) {
      std::move(state->promise).Set(std::move(exp.value()));
    } else if (!exp && state->state.exchange(ProtectedPromise::ReportError) ==
                           ProtectedPromise::ReportError) {
      std::move(state->promise).Set(std::unexpected(std::move(exp.error())));
    }
  };

  std::move(f1).Consume(cb);
  std::move(f2).Consume(cb);

  return std::move(future);
}

}  // namespace exe::future
