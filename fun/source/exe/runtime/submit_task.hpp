#pragma once

#include <exe/runtime/view/tasks.hpp>

namespace exe::runtime {

inline void SubmitTask(View rt, task::Task task) {
  Tasks(rt).Submit(std::move(task));
}

}  // namespace exe::runtime
