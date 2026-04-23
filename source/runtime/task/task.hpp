#pragma once

#include <vvv/list.hpp>

namespace exe::runtime::task {

struct ITask {
  virtual void Run() noexcept = 0;

 protected:
  ~ITask() = default;

 private:
};

struct TaskBase : ITask,
                  vvv::IntrusiveListNode<TaskBase> {};

}  // namespace exe::runtime::task
