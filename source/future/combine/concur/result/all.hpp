#pragma once

#include <exe/future/type/result.hpp>
#include <exe/future/detail/manual_storage.hpp>

#include <memory>
#include <tuple>

namespace exe::future {

template <typename A, typename B, typename E>
TryFuture<std::tuple<A, B>, E> BothOk(TryFuture<A, E> f1, TryFuture<B, E> f2) {
  auto [future, promise] = Contract<Try<std::tuple<A, B>, E>>();

  struct ProtectedPromise {
    detail::ManualStorage<A> v1;
    detail::ManualStorage<B> v2;

    Promise<Try<std::tuple<A, B>, E>> promise;

    twist::ed::std::atomic_int state = None;
    enum : int { None = 0, First = 1, Second = 2, Both = 3, Fail = 4 };

    explicit ProtectedPromise(Promise<Try<std::tuple<A, B>, E>> promise)
        : promise(std::move(promise)) {
    }
  };

  auto state = std::make_shared<ProtectedPromise>(std::move(promise));

  std::move(f1).Consume([state](Try<A, E> v1) mutable {
    if (!v1) {
      std::move(state->promise).Set(std::unexpected(std::move(v1.error())));
      state->state.fetch_or(ProtectedPromise::Fail);
      return;
    }

    state->v1.Emplace(std::move(v1.value()));
    if (state->state.fetch_or(ProtectedPromise::First) ==
        ProtectedPromise::Second) {
      std::move(state->promise)
          .Set(std::make_tuple(std::move(*state->v1), std::move(*state->v2)));
    }
  });

  std::move(f2).Consume([state](Try<B, E> v2) mutable {
    if (!v2) {
      std::move(state->promise).Set(std::unexpected(std::move(v2.error())));
      state->state.fetch_or(ProtectedPromise::Fail);
      return;
    }

    state->v2.Emplace(std::move(v2.value()));
    if (state->state.fetch_or(ProtectedPromise::Second) ==
        ProtectedPromise::First) {
      std::move(state->promise)
          .Set(std::make_tuple(std::move(*state->v1), std::move(*state->v2)));
    }
  });

  return std::move(future);
}

}  // namespace exe::future
