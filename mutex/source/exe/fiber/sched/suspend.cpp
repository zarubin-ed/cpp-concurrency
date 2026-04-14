#include "suspend.hpp"
#include <exe/fiber/core/fiber.hpp>

namespace exe::fiber {

void Suspend(Callback cb) {
  Fiber::Self().Suspend(std::move(cb));
}

}  // namespace exe::fiber