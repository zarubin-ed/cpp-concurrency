#pragma once

#include <twist/ed/sure/stack/mmap.hpp>

namespace exe::fiber {

using Stack = twist::ed::sure::stack::GuardedMmapExecutionStack;

}  // namespace exe::fiber
