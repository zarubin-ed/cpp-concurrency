#pragma once

#include "callback.hpp"
#include "rendezvous.hpp"
#include "manual_storage.hpp"

#include <exe/runtime/view.hpp>
#include <exe/runtime/inline.hpp>

namespace exe::future {

namespace detail {

// Future state for future::Contract

template <typename T>
class ContractState {
  using Error = std::exception_ptr;

 public:
  void Produce(T res) {
    result_.Emplace(std::move(res));
    if (state_processor_.Produce()) {
      Rendezvous();
    }
  }

  void Consume(Callback<T> cb) {
    callback_.Emplace(std::move(cb));
    if (state_processor_.Consume()) {
      Rendezvous();
    }
  }

  void SetRuntime(runtime::View runtime) {
    runtime_ = runtime;
  }

  runtime::View GetRuntime() {
    return runtime_;
  }

 private:
  void Rendezvous() {
    (*callback_)(std::move(*result_));
    delete this;
  }

  RendezvousStateMachine state_processor_;

  ManualStorage<Callback<T>> callback_;
  ManualStorage<T> result_;

  runtime::View runtime_{runtime::Inline()};
};

}  // namespace detail

}  // namespace exe::future
