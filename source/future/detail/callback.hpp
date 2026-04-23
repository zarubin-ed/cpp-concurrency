#pragma once

#include <function2/function2.hpp>

namespace exe::future {

namespace detail {

template <typename T>
using Callback = fu2::unique_function<void(T)>;

}  // namespace detail

}  // namespace exe::future
