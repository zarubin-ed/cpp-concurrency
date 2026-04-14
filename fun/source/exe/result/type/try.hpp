#pragma once

#include "result.hpp"

namespace exe {

// Synonym for Result<T, E>

template <typename T, typename E>
using Try = Result<T, E>;

}  // namespace exe
