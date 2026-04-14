#include "yield.hpp"
#include "suspend.hpp"

namespace exe::fiber {

void Yield() {
  Suspend([](FiberHandle handle) {
    handle.Schedule();
  });
}

}  // namespace exe::fiber