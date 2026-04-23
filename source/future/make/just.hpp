#pragma once

#include <exe/future/make/value.hpp>

#include <exe/unit.hpp>

namespace exe::future {

/*
 * Ready unit value
 *
 * https://en.wikipedia.org/wiki/Unit_type
 *
 * Usage:
 *
 * auto f = future::Just()
 *          | future::Via(runtime)
 *          | future::Map([](Unit) { return 5; });
 *
 */

inline Future<Unit> Just() {
  return Value(unit);
}

}  // namespace exe::future
