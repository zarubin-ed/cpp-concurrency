#include "collector.hpp"
#include "mutator.hpp"

namespace gc::hazard {

void Mutator::Gc() {
  vvv::IntrusiveList<Deleter> kept;

  while (auto* obj = retired_.TryPopFront()) {
    if (collector_->IsProtected(obj)) {
      kept.PushBack(obj);
    } else {
      obj->Destroy();
    }
  }

  retired_.Append(kept);
}

}  // namespace gc::hazard