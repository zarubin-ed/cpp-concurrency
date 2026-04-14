#pragma once

#include <exe/runtime/view.hpp>
#include <exe/runtime/task/scheduler.hpp>

namespace exe::runtime {

task::IScheduler& Tasks(View);

}  // namespace exe::runtime
