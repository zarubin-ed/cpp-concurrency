#pragma once

#include <exe/future/type/result.hpp>

#include <exe/thread/spinlock.hpp>
#include <exe/thread/unique_lock.hpp>

#include <cstdlib>  // std::abort

namespace exe::future {

// First success / last error

template <typename T, typename E>
TryFuture<T, E> FirstOk(TryFuture<T, E> f1, TryFuture<T, E> f2) {
  auto [future, promise] = Contract<Try<T, E>>();

  struct ProtectedPromise {
    enum : int { Satisfied = 1, ReportError = 2, ReturnValue = 0 };

    explicit ProtectedPromise(Promise<Try<T, E>> p)
        : promise(std::move(p)) {
    }

    int state{ReturnValue};
    thread::SpinLock spin;
    Promise<Try<T, E>> promise;
  };

  std::shared_ptr<ProtectedPromise> state =
      std::make_shared<ProtectedPromise>(std::move(promise));

  auto cb = [state](Try<T, E> exp) {
    thread::UniqueLock lock(state->spin);

    if (state->state == ProtectedPromise::Satisfied) {
      return;
    }

    if (exp) {
      state->state = ProtectedPromise::Satisfied;
      std::move(state->promise).Set(std::move(exp.value()));
    } else {
      if (state->state == ProtectedPromise::ReportError) {
        std::move(state->promise).Set(std::unexpected(std::move(exp.error())));
      } else {
        state->state = ProtectedPromise::ReportError;
      }
    }
  };

  std::move(f1).Consume(cb);
  std::move(f2).Consume(cb);

  return std::move(future);
}

}  // namespace exe::future
