#pragma once

#include <exe/fiber/core/body.hpp>
#include <exe/runtime/view.hpp>

namespace exe::fiber {

// Considered harmful

void Go(runtime::View, Body);

void Go(Body);

}  // namespace exe::fiber