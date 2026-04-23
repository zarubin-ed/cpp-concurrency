#pragma once

#include <exe/future/type/future.hpp>
#include <exe/future/detail/manual_storage.hpp>

#include <memory>
#include <tuple>

namespace exe::future {

template <typename A, typename B>
Future<std::tuple<A, B>> Both(Future<A> f1, Future<B> f2) {
  auto [future, promise] = Contract<std::tuple<A, B>>();

  struct ProtectedPromise {
    detail::ManualStorage<A> v1;
    detail::ManualStorage<B> v2;

    Promise<std::tuple<A, B>> promise;

    twist::ed::std::atomic_int state{None};
    enum : int { None = 0, First = 1, Second = 2, Both = 3 };

    explicit ProtectedPromise(Promise<std::tuple<A, B>> promise)
        : promise(std::move(promise)) {
    }
  };

  auto state = std::make_shared<ProtectedPromise>(std::move(promise));

  std::move(f1).Consume([state](A v1) mutable {
    state->v1.Emplace(std::move(v1));

    if (state->state.fetch_or(ProtectedPromise::First) ==
        ProtectedPromise::Second) {
      std::move(state->promise)
          .Set(std::make_tuple(std::move(*state->v1), std::move(*state->v2)));
    }
  });

  std::move(f2).Consume([state](B v2) mutable {
    state->v2.Emplace(std::move(v2));

    if (state->state.fetch_or(ProtectedPromise::Second) ==
        ProtectedPromise::First) {
      std::move(state->promise)
          .Set(std::make_tuple(std::move(*state->v1), std::move(*state->v2)));
    }
  });

  return std::move(future);
}

}  // namespace exe::future
