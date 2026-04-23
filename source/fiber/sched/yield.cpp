#include "yield.hpp"
#include "suspend.hpp"

namespace exe::fiber {

void Yield() {
  auto callback = [](FiberHandle handle) {
    handle.Schedule();
  };

  Suspend(Callback(callback));
}

}  // namespace exe::fiber