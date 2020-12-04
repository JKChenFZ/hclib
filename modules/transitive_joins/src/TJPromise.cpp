#include "TJPromise.h"

namespace hclib {
namespace transitivejoins {

TaskNode* TJPromiseBase::getOwnerTaskNode() {
    return ownerTaskNode_; 
}

void TJPromiseBase::setNewOwnerTaskNode(TaskNode* newOwnerTaskNode) {
    ownerTaskNode_ = newOwnerTaskNode;
}

void TJPromiseBase::addDependencyNode(TaskNode* newDependencyNode) {
#ifdef ENABLE_PROMISE_LCA
    std::lock_guard<std::mutex> lock(dependencyNodesLock);
    dependencyNodes.push_back(newDependencyNode);
#endif // ENABLE_PROMISE_LCA
}

void TJPromiseBase::signalAllDependencyNodes() {
#ifdef ENABLE_PROMISE_LCA
    std::lock_guard<std::mutex> lock(dependencyNodesLock);
    for (auto nodePtr : dependencyNodes) {
        if (ownerTaskNode_ == nodePtr) {
            assert(nodePtr->edgeCount < 0 && "Undoing a lesser waits on greater edge");
            nodePtr->edgeCount++;
        } else {
            assert(nodePtr->edgeCount > 0 && "Undoing a greater waits on lesser edge");
            nodePtr->edgeCount--;
        }
    }
    dependencyNodes.clear();
#endif // ENABLE_PROMISE_LCA
}

} // namespace transitivejoins
} // namespace hclib