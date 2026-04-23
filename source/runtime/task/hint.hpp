#pragma once

namespace exe::runtime::task {

enum class SchedulingHint {
  UpToYou = 1,  // Rely on scheduler decision
  Next = 2,     // Use LIFO scheduling
  Yield = 3,    // Yield control to another task chain
};

}  // namespace exe::runtime::task
