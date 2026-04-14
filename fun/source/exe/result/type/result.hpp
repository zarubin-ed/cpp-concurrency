#pragma once

#include <expected>

namespace exe {

template <typename T, typename E>
using Result = std::expected<T, E>;

}  // namespace exe
