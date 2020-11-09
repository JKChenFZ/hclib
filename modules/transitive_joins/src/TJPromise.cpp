#include "TJPromise.h"

namespace hclib {
namespace transitivejoins {

void TJPromiseBase::setNewOwner(TaskNode* newOwnerNode) {
    ownerTaskNode_ = newOwnerNode;
}

TaskNode* TJPromiseBase::getOwnerTaskNode() {
    return ownerTaskNode_; 
}

void TJPromiseBase::addDependencyNode(TaskNode* newDependencyNode) {
    std::lock_guard<std::mutex> lock(dependencyNodesLock);
    dependencyNodes.push_back(newDependencyNode);
}

void TJPromiseBase::signalAllDependencyNodes() {
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
}

} // namespace transitivejoins
} // namespace hclib