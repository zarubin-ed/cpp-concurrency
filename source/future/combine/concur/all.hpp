#pragma once

#include <exe/future/type/future.hpp>
#include <exe/future/detail/manual_storage.hpp>

#include <exe/thread/spinlock.hpp>
#include <exe/thread/unique_lock.hpp>

#include <memory>
#include <tuple>

namespace exe::future {

template <typename A, typename B>
Future<std::tuple<A, B>> Both(Future<A> f1, Future<B> f2) {
  auto [future, promise] = Contract<std::tuple<A, B>>();

  struct ProtectedPromise {
    detail::ManualStorage<A> v1;
    detail::ManualStorage<B> v2;
    thread::SpinLock spin;
    enum : int { None = 0, First = 1, Second = 2, Both = 3 };
    int state = None;
    Promise<std::tuple<A, B>> promise;

    explicit ProtectedPromise(Promise<std::tuple<A, B>> promise)
        : promise(std::move(promise)) {
    }
  };

  auto state = std::make_shared<ProtectedPromise>(std::move(promise));

  std::move(f1).Consume([state](A v1) mutable {
    thread::UniqueLock lock(state->spin);
    state->state |= ProtectedPromise::First;
    state->v1.Emplace(std::move(v1));
    if (state->state == ProtectedPromise::Both) {
      std::move(state->promise)
          .Set(std::make_tuple(std::move(*state->v1), std::move(*state->v2)));
    }
  });

  std::move(f2).Consume([state](B v2) mutable {
    thread::UniqueLock lock(state->spin);
    state->state |= ProtectedPromise::Second;
    state->v2.Emplace(std::move(v2));
    if (state->state == ProtectedPromise::Both) {
      std::move(state->promise)
          .Set(std::make_tuple(std::move(*state->v1), std::move(*state->v2)));
    }
  });

  return std::move(future);
}

}  // namespace exe::future
