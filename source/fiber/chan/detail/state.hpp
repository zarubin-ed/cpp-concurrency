#pragma once

#include <exe/fiber/core/handle.hpp>

#include <vvv/list.hpp>

namespace exe::fiber {

namespace detail {

template <typename T>
struct WaiterNode : vvv::IntrusiveListNode<WaiterNode<T>> {
  FiberHandle handle;
  T* message = nullptr;
};

struct States {
  enum : uint32_t { Nobody = 0, Senders = 1, Receivers = 2 };
};

}  // namespace detail

}  // namespace exe::fiber
