#include "inline.hpp"

#include <exe/runtime/task/scheduler.hpp>

namespace exe::runtime {

namespace {

struct InlineExecutor final : task::IScheduler {
  void Submit(task::Task task) override {
    task();
  }
};

}  // namespace

View Inline() {
  static InlineExecutor executor;
  return View{&executor, nullptr};
}

}  // namespace exe::runtime
