#include "TaskNode.h"

#include <iostream>

#include "hclib_cpp.h"

namespace hclib {
namespace transitivejoins {
//////////////////////////////////////////////////////////////////////////////
// TaskNode Class Implementation
//////////////////////////////////////////////////////////////////////////////
TaskNode::TaskNode() {
    siblingOrder_ = 1;
    depth_ = 1;

#ifdef LOG
    nodeID_ = rand();
    workerThreadID_ = current_ws()->id;

    std::cout << "[" << nodeID_ << "] Root Task created, running on " << workerThreadID_ << std::endl;
#endif
}

TaskNode::TaskNode(TaskNode* parentTaskNode) {
    siblingOrder_ = parentTaskNode->getSiblingOrderForNewChild();
    parentTaskNode_ = parentTaskNode;
    depth_ = parentTaskNode->getDepth() + 1;

#ifdef LOG
    nodeID_ = rand();
    workerThreadID_ = current_ws()->id;

    std::cout << "[" << nodeID_ << "] Task created, parent is " << parentTaskNode_->getNodeID() << std::endl;
    std::cout << "[" << nodeID_ << "] Sibling order is " << siblingOrder_ << ", running on " << workerThreadID_ << std::endl;
    std::cout << "[" << nodeID_ << "] Depth is " << depth_ << std::endl;
#endif
}

TaskNode::~TaskNode() {
#ifdef LOG
    std::cout << "[" << nodeID_ << "] Task deleted on " << workerThreadID_ << " ." << std::endl;
#endif
}

TaskNode* TaskNode::getParentNode() {
    return parentTaskNode_;
}

uint32_t TaskNode::getSiblingOrder() {
    return siblingOrder_;
}

uint32_t TaskNode::getNumChildren() {
    return numChildren_;
}

uint32_t TaskNode::getDepth() {
    return depth_;
}

uint32_t TaskNode::getSiblingOrderForNewChild() {
    return ++numChildren_;
}

#ifdef LOG
uint32_t TaskNode::getNodeID() {
    return nodeID_;
}
#endif

void TaskNode::removeTJPromise(TJPromiseBase* promiseToRemove) {
#ifdef ENABLE_PROMISE_FULFILLMENT_CHECK
    ownedPromises.erase(promiseToRemove);
#endif // ENABLE_PROMISE_FULFILLMENT_CHECK
}

void TaskNode::addNewTJPromise(TJPromiseBase* newPromise) {
#ifdef ENABLE_PROMISE_FULFILLMENT_CHECK
    ownedPromises.insert(newPromise);
#endif // ENABLE_PROMISE_FULFILLMENT_CHECK
}

} // namespace transitivejoins
} // namespace hclib