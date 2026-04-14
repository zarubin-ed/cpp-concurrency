#pragma once

#include <exe/future/type/result.hpp>
#include <exe/future/detail/manual_storage.hpp>

#include <exe/thread/spinlock.hpp>
#include <exe/thread/unique_lock.hpp>

#include <memory>
#include <tuple>

namespace exe::future {

template <typename A, typename B, typename E>
TryFuture<std::tuple<A, B>, E> BothOk(TryFuture<A, E> f1, TryFuture<B, E> f2) {
  auto [future, promise] = Contract<Try<std::tuple<A, B>, E>>();

  struct ProtectedPromise {
    detail::ManualStorage<A> v1;
    detail::ManualStorage<B> v2;
    thread::SpinLock spin;
    enum : int { None = 0, First = 1, Second = 2, Both = 3, Fail = -1 };
    int state = None;
    Promise<Try<std::tuple<A, B>, E>> promise;

    explicit ProtectedPromise(Promise<Try<std::tuple<A, B>, E>> promise)
        : promise(std::move(promise)) {
    }
  };

  auto state = std::make_shared<ProtectedPromise>(std::move(promise));

  std::move(f1).Consume([state](Try<A, E> v1) mutable {
    thread::UniqueLock lock(state->spin);
    if (state->state == ProtectedPromise::Fail) {
      return;
    }
    state->state |= ProtectedPromise::First;
    if (!v1) {
      std::move(state->promise).Set(std::unexpected(std::move(v1.error())));
      state->state = ProtectedPromise::Fail;
      return;
    }
    state->v1.Emplace(std::move(v1.value()));
    if (state->state == ProtectedPromise::Both) {
      std::move(state->promise)
          .Set(std::make_tuple(std::move(*state->v1), std::move(*state->v2)));
    }
  });

  std::move(f2).Consume([state](Try<B, E> v2) mutable {
    thread::UniqueLock lock(state->spin);
    if (state->state == ProtectedPromise::Fail) {
      return;
    }
    state->state |= ProtectedPromise::Second;
    if (!v2) {
      std::move(state->promise).Set(std::unexpected(std::move(v2.error())));
      state->state = ProtectedPromise::Fail;
      return;
    }
    state->v2.Emplace(std::move(v2.value()));
    if (state->state == ProtectedPromise::Both) {
      std::move(state->promise)
          .Set(std::make_tuple(std::move(*state->v1), std::move(*state->v2)));
    }
  });

  return std::move(future);
}

}  // namespace exe::future
