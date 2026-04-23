#pragma once
#include <exe/fiber/core/callback.hpp>

namespace exe::fiber {

class WaitQueue;

void Suspend(Callback);

}  // namespace exe::fiber