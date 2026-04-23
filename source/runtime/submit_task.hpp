#pragma once

#include <exe/runtime/task/boxed.hpp>
#include <exe/runtime/view/tasks.hpp>

namespace exe::runtime {

template <typename F>
void SubmitTask(View rt, F task) {
  Tasks(rt).Submit(new task::Boxed(std::move(task)));
}

}  // namespace exe::runtime
