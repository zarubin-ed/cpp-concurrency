#pragma once

#include <exe/runtime/task/fwd.hpp>
#include <exe/runtime/timer/fwd.hpp>

#include <tuple>

namespace exe::runtime {

// clang-format off

using View = std::tuple<
    task::IScheduler*,
    timer::IScheduler*
    >;

// clang-format on

}  // namespace exe::runtime
