#pragma once

#include <chrono>

namespace exe::fiber {

void SleepFor(std::chrono::microseconds delay);

}  // namespace exe::fiber
