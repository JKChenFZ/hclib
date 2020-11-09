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

uint32_t TaskNode::getNodeID() {
#ifdef LOG
    return nodeID_;
#else
    return 0;
#endif
}

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

TaskNode* getCurrentTaskNode() {
#ifdef ENABLE_TASK_TREE
    void** currTaskLocalPtr = hclib_get_curr_task_local();

    if (!((TaskNode*)(*currTaskLocalPtr))) {
        // Create node for root task "hclib::launch/finish"
#ifdef LOG
        std::cout << "[Runtime API] TaskNode Created for root task" << std::endl;
#endif
        *currTaskLocalPtr = new TaskNode();
    }

    return (TaskNode*)(*currTaskLocalPtr);
#else
    return nullptr;
#endif // ENABLE_TASK_TREE
}

void setUpTaskNode(TaskNode* currTaskNode) {
#ifdef ENABLE_TASK_TREE
    *(hclib_get_curr_task_local()) = currTaskNode;
#endif // ENABLE_TASK_TREE
}

} // namespace transitivejoins
} // namespace hclib