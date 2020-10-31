#pragma once

#include "TaskNode.h"
#include "TJFuture.h"
#include "TJPromise.h"
#include "TJPromFuture.h"

namespace hclib {
namespace transitivejoins {
//////////////////////////////////////////////////////////////////////////////
// Runtime API Declarations
//////////////////////////////////////////////////////////////////////////////
TaskNode* getCurrentTaskNode();

// Invoke from Future
void verifyFutureWaitFor(TJFutureBase* dependencyFuture);

// Invoke from Future as well
void verifyPromiseWaitFor(TJPromiseBase* dependencyPromise);

// Invoke from Promise on fulfillment
void promiseLCASignalDependencyEdges(TJPromiseBase* promise);

// Invoke from Async APIs
void verifyFutureAwaitWithLCA(TaskNode* currTaskNode, TJFutureBase* dependencyFuture);

void setUpTaskNode(TaskNode* currTaskNode);

TaskNode* generateNewTaskNode();

template<class T>
auto getNewTJPromise() {
    auto newPromise = new TJPromise<T>();
    newPromise->setNewOwner(getCurrentTaskNode());

    // insert the new promise into the owner TaskNode
    getCurrentTaskNode()->addNewTJPromise(newPromise);

    return newPromise;
}

template <typename T>
void transferPromiseOwnership(TJPromise<T>* promiseToTransfer, TaskNode* newTaskNode) {
    // In the case of not generating nodes, we don't perform transfer
    if (!newTaskNode) {
        return;
    }

    if (promiseToTransfer->getOwnerTaskNode() != getCurrentTaskNode()) {
        throw std::runtime_error("A promise can not be transferred twice!");
    }

    promiseToTransfer->getOwnerTaskNode()->removeTJPromise(promiseToTransfer);
    promiseToTransfer->setNewOwner(newTaskNode);
    newTaskNode->addNewTJPromise(promiseToTransfer);
}

//////////////////////////////////////////////////////////////////////////////
// TJPromise Fulfillment Scope Guard Declarations
//////////////////////////////////////////////////////////////////////////////
struct TJPromiseFulfillmentScopeGuard {
    TJPromiseFulfillmentScopeGuard(TaskNode* currentScopeTaskNode);
    ~TJPromiseFulfillmentScopeGuard() noexcept(false);

private:
    TaskNode* currentScopeTaskNode_;
};

} // namespace transitivejoins
} // namespace hclib