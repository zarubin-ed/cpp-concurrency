#include "handle.hpp"

#include <function2/function2.hpp>

namespace exe::fiber {

using Callback = fu2::unique_function<void(FiberHandle)>;

}  // namespace exe::fiber