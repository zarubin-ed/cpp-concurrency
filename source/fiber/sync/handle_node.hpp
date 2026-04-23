#include <vvv/list.hpp>
#include <exe/fiber/core/handle.hpp>

namespace exe::fiber {

class FiberHandleNode : public vvv::IntrusiveListNode<FiberHandleNode> {
 public:
  FiberHandleNode() = default;

  explicit FiberHandleNode(FiberHandle handle)
      : handle_(handle) {};

  FiberHandle& GetHandle() {
    return handle_;
  }

 private:
  FiberHandle handle_;
};

}  // namespace exe::fiber
