#include "TJFuture.h"

namespace hclib {
namespace transitivejoins {

TaskNode* TJFutureBase::getOwnerTaskNode() {
    return ownerTaskNode_;
}

} // namespace transitivejoins
} // namespace hclib