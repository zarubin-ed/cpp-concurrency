#include "sleep_for.hpp"
#include "suspend.hpp"

namespace exe::fiber {

void SleepFor(std::chrono::microseconds delay) {
  Suspend([delay](FiberHandle handle) {
    handle.Schedule(delay);
  });
}

}  // namespace exe::fiber
