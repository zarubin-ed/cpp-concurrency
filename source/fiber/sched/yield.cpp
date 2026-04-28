#include "yield.hpp"
#include "suspend.hpp"

namespace exe::fiber {

void Yield() {
  auto callback = [](FiberHandle handle) {
    handle.Schedule(runtime::task::SchedulingHint::Yield);
  };

  Suspend(Callback(callback));
}

}  // namespace exe::fiber