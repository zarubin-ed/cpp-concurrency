#pragma once
// #define CUSTOM_PRIMITIVES

#ifdef CUSTOM_PRIMITIVES

#include "mutex.hpp"
#include "condvar.hpp"

using CondVar = primitives::CondVar;
using Mutex = primitives::Mutex;

#else

#include <twist/ed/std/mutex.hpp>
#include <twist/ed/std/condition_variable.hpp>

using CondVar = twist::ed::std::condition_variable;
using Mutex = twist::ed::std::mutex;

#endif