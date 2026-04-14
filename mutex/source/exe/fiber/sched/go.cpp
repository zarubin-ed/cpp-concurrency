#include "go.hpp"

#include <exe/fiber/core/fiber.hpp>

namespace exe::fiber {

void Go(runtime::View runtime, Body func) {
  new Fiber(runtime, std::move(func));
}

void Go(Body func) {
  Fiber::Self().Go(std::move(func));
}

}  // namespace exe::fiber