#pragma once

#include "TaskNode.h"
#include "TJFuture.h"
#include "TJPromise.h"

namespace hclib {
namespace transitivejoins {
/*
 * Execute LCA algorithm to verify a wait-for relationship between
 * current executing TaskNode and the TaskNode associated with
 * the parameter "dependencyPromise"
 */
void verifyPromiseWaitFor(TJPromiseBase* dependencyPromise);

/*
 * Execute LCA algorithm to verify a wait-for relationship between
 * current executing TaskNode and the TaskNode associated with
 * the parameter "dependencyFuture"
 */
void verifyFutureWaitFor(TJFutureBase* dependencyFuture);

/*
 * Execute LCA algorithm to verify a wait-for relationship between
 * current executing TaskNode and the TaskNode associated with
 * the parameter "dependencyFuture"
 * 
 * Invoked through implicit wait-for using async_await
 */
void verifyFutureAwaitWithLCA(TaskNode* currTaskNode, TJFutureBase* dependencyFuture);

/*
 * Transfers the ownership ("ownerTaskNode") from current node to the given
 * "newTaskNode"
 */
template <typename T>
void transferPromiseOwnership(TJPromise<T>* promiseToTransfer, TaskNode* newTaskNode) {
    auto currentNode = getCurrentTaskNode();
    if (!newTaskNode || !currentNode) {
        return;
    }

    if (promiseToTransfer->getOwnerTaskNode() != currentNode) {
        throw std::runtime_error("Current TaskNode does not own this promise");
    }

    promiseToTransfer->getOwnerTaskNode()->removeTJPromise(promiseToTransfer);
    promiseToTransfer->setNewOwner(newTaskNode);
    newTaskNode->addNewTJPromise(promiseToTransfer);
}

/*
 * A scope guard to check whether owned promises of each task
 * are fullfilled at the end of the task execution
 * 
 * Bookkeeping and verification are enabled only if
 * 'ENABLE_PROMISE_FULFILLMENT_CHECK' flag is enabled in compilation
 * of transitive joins
 */
struct TJPromiseFulfillmentScopeGuard {
    TJPromiseFulfillmentScopeGuard(TaskNode* currentScopeTaskNode);
    ~TJPromiseFulfillmentScopeGuard() noexcept(false);

private:
    TaskNode* currentScopeTaskNode_;
};

} // namespace transitivejoins
} // namespace hclib