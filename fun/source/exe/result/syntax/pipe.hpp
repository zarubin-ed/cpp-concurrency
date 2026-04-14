#pragma once

#include <exe/result/type/result.hpp>

template <typename T, typename E, typename Combinator>
auto operator|(exe::Result<T, E> r, Combinator c) {
  return c.Pipe(std::move(r));
}
