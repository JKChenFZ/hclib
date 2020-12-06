#include "VerificationAPI.h"

#include <assert.h>
#include <atomic>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <tuple>

#include "hclib_cpp.h"

namespace hclib {
namespace transitivejoins {
namespace {

static std::atomic<size_t> numNodes {0};
static std::atomic<size_t> numFutureLCA {0};
static std::atomic<size_t> numPromiseLCA {0};

std::tuple<TaskNode*, TaskNode*> alignNodeDepth(
    TaskNode* initiatorNodePtr,
    TaskNode* dependencyNodePtr
) {
    uint32_t initiatorDepth = initiatorNodePtr->getDepth();
    uint32_t dependencyDepth = dependencyNodePtr->getDepth();

    if (initiatorDepth == dependencyDepth) {
        // No alignment needed
        return std::make_tuple(initiatorNodePtr, dependencyNodePtr);
    } else {
        if (initiatorDepth > dependencyDepth) {
            while (initiatorDepth > dependencyDepth) {
                initiatorNodePtr = initiatorNodePtr->getParentNode();
                initiatorDepth--;
            }
        } else {
            while (initiatorDepth < dependencyDepth) {
                dependencyNodePtr = dependencyNodePtr->getParentNode();
                dependencyDepth--;
            }
        }
    }

    return std::make_tuple(initiatorNodePtr, dependencyNodePtr);
}

bool verifySiblingOrder(TaskNode* initiatorNodePtr, TaskNode* dependencyNodePtr) {
    TaskNode* initiatorNodePrePtr = nullptr;
    TaskNode* dependencyNodePrePtr = nullptr;

    while (initiatorNodePtr != dependencyNodePtr) {
        // Remember the previous level as we climb up
        initiatorNodePrePtr = initiatorNodePtr;
        dependencyNodePrePtr = dependencyNodePtr;

        // Advance into next level (parents)
        initiatorNodePtr = initiatorNodePtr->getParentNode();
        dependencyNodePtr = dependencyNodePtr->getParentNode();
    }

    // For futures, only younger ones can wait on older ones
    if (initiatorNodePrePtr->getSiblingOrder() > dependencyNodePrePtr->getSiblingOrder()) {
#ifdef LOG
        std::cout << "[Runtime API] Valid wait for relationship detected" << std::endl;
        std::cout << "[Runtime API] Initiator Node with ID: " << initiatorNodePrePtr->getNodeID()
                  << " and sibling order of " << initiatorNodePrePtr->getSiblingOrder() << std::endl;
        std::cout << "[Runtime API] Dependency Node with ID: " << dependencyNodePrePtr->getNodeID()
                  << " and sibling order of " << dependencyNodePrePtr->getSiblingOrder() << std::endl;
#endif
        return true;
    } else {
#ifdef LOG
        std::cout << "[Runtime API] Invalid wait for relationship detected" << std::endl;
        std::cout << "[Runtime API] Initiator Node with ID: " << initiatorNodePrePtr->getNodeID()
                  << " and sibling order of " << initiatorNodePrePtr->getSiblingOrder() << std::endl;
        std::cout << "[Runtime API] Dependency Node with ID: " << dependencyNodePrePtr->getNodeID()
                  << " and sibling order of " << dependencyNodePrePtr->getSiblingOrder() << std::endl;
#endif
        return false;
    }
}

bool verifyFutureWaitForImp(TaskNode* initiatorNodePtr, TJFutureBase* dependencyFuture) {
    TaskNode* dependencyNodePtr = dependencyFuture->getOwnerTaskNode();
#ifdef LOG
    std::cout << "[Runtime API] A wait-for request has been captured, ";
    std::cout << initiatorNodePtr->getNodeID() << " waits on " << dependencyNodePtr->getNodeID();
    std::cout << std::endl;
#endif

    if (!initiatorNodePtr || !dependencyNodePtr) {
        throw std::runtime_error("[Runtime API] Cannot verify wait for relationship with nullptr");
    }

    if (initiatorNodePtr == dependencyNodePtr) {
        throw std::runtime_error("[Runtime API] Wait for relationship can not be reflexive");
    }

    // Align by TaskNode Depth
    auto alignmentResult = alignNodeDepth(initiatorNodePtr, dependencyNodePtr);

    // Check to see if future was fulfilled again
    if (dependencyFuture->isFulfilled()) {
        return true;
    }

    TaskNode* alignedInitiatorNodePtr = std::get<0>(alignmentResult);
    TaskNode* alignedDependencyNodePtr  = std::get<1>(alignmentResult);

    // check if both pointers equal to each other
    // if so, both node exists in the same task node sub tree and one
    // is child (might be transitive) of the other
    if (alignedInitiatorNodePtr == alignedDependencyNodePtr) {
        if (initiatorNodePtr->getDepth() < dependencyNodePtr->getDepth()) {
            // Initialtor node is an ancester of the dependency node
            // this relationship is always a valid for-wait
#ifdef LOG
            std::cout << "[Runtime API] Verifier detected a parent waits on child relationship" << std::endl;
#endif
            return true;
        } else {
            // Initiator node is a child of the dependency node
            // this relationship is always invalid
#ifdef LOG
            std::cout << "[Runtime API] Verifier detected a child waits on parent relationship" << std::endl;
#endif
            return false;
        }
    }

    // Both node exist on different Task node subtrees
    // Validate by comparing the sibling order
    return verifySiblingOrder(alignedInitiatorNodePtr, alignedDependencyNodePtr);
}

//////////////////////////////////////////////
// Helper methods for Promise LCA verification
//////////////////////////////////////////////
void greaterNodeWaitingOnLesserNode(
    TaskNode* initiatorNodePtr,
    TaskNode* depedencyNodePtr,
    TJPromiseBase* dependencyPromise) {
    // The heuristic is that when a greater node waits on a lesser node,
    // the edge counter in greater (initiatorNode) should be incremeted.
    // Note: If edge count is negative, concave detected, we throw

    // The lesser node (dependencyNode) will remember the greater node.
    // On fulfillment of the dependencyPromise, low will come back to high
    // and decrement the count
    if (dependencyPromise->isFulfilled()) {
        // if already fulfilled, we skip
        return;
    }

    // Atomic fetch and add returns the previous value
    if (std::atomic_fetch_add(&(initiatorNodePtr->edgeCount), 1) < 0) {
        // Concave turn detected, throw error
        throw std::runtime_error("LCA Verification failed, see log");
    } else {
        dependencyPromise->addDependencyNode(initiatorNodePtr);

        // There is a chance that the promise was fulfilled while the dependency was being added
        // we will check that here and immediately undo the edge
        if (dependencyPromise->isFulfilled()) {
            dependencyPromise->signalAllDependencyNodes();
        }
    }
}

void lesserNodeWaitingOnGreaterNode(
    TaskNode* initiatorNodePtr,
    TaskNode* depedencyNodePtr,
    TJPromiseBase* dependencyPromise) {
    // The heuristic is that when a lesser node waits on a greater node,
    // the edge counter in greater (depedencyNodePtr) should be decremented.
    // Note: If edge count is positive, concave detected, we throw

    // The greated node (depedencyNodePtr) will remember itself.
    // On fulfillment of the dependencyPromise, greater node will come back
    // and increment its own count
    if (dependencyPromise->isFulfilled()) {
        // if already fulfilled, we skip
        return;
    }

    // Atomic fetch and sub returns the previous value
    if (std::atomic_fetch_sub(&(depedencyNodePtr->edgeCount), 1) > 0) {
        // Concave turn detected, throw error
        throw std::runtime_error("LCA Verification failed, see log");
    } else {
        dependencyPromise->addDependencyNode(depedencyNodePtr);

        // There is a chance that the promise was fulfilled while the dependency was being added
        // we will check that here and immediately undo the edge
        if (dependencyPromise->isFulfilled()) {
            dependencyPromise->signalAllDependencyNodes();
        }
    }
}

void verifyPromiseWaitForImp(TaskNode* initiatorNode, TJPromiseBase* dependencyPromise) {
    TaskNode* dependencyNode = dependencyPromise->getOwnerTaskNode();
    // Find the LCA reps
    // Remember the previous pointers, as we climb up
    TaskNode* initiatorNodePrePtr = initiatorNode;
    TaskNode* dependencyNodePrePtr = dependencyNode;

    TaskNode* initiatorNodeCurrPtr = initiatorNode;
    TaskNode* dependencyNodeCurrPtr = dependencyNode;

    // Align the depth
    while (initiatorNodeCurrPtr->getDepth() != dependencyNodeCurrPtr->getDepth()) {
        if (initiatorNodeCurrPtr->getDepth() > dependencyNodeCurrPtr->getDepth()) {
            initiatorNodePrePtr = initiatorNodeCurrPtr;
            initiatorNodeCurrPtr = initiatorNodeCurrPtr->getParentNode();
        } else {
            dependencyNodePrePtr = dependencyNodeCurrPtr;
            dependencyNodeCurrPtr = dependencyNodeCurrPtr->getParentNode();
        }
    }

    if (initiatorNodeCurrPtr == dependencyNodeCurrPtr) {
#ifdef LOG
        std::cout << "[PromiseLCA] Found parent child wait-for";
        std::cout << " " << initiatorNode->getNodeID() << " waits on " << dependencyNode->getNodeID() << std::endl;
#endif
        if (initiatorNodeCurrPtr == initiatorNodePrePtr) {
            // initiator node did not move, implies that it is the parent node
            // parent has the lowest order
            lesserNodeWaitingOnGreaterNode(initiatorNodeCurrPtr, dependencyNodePrePtr, dependencyPromise);
        } else {
            greaterNodeWaitingOnLesserNode(initiatorNodePrePtr, dependencyNodeCurrPtr, dependencyPromise);
        }

        // No more LCA needed
        return;
    }

    // non parent-child relation
    while (initiatorNodeCurrPtr != dependencyNodeCurrPtr) {
        initiatorNodePrePtr = initiatorNodeCurrPtr;
        initiatorNodeCurrPtr = initiatorNodeCurrPtr->getParentNode();

        dependencyNodePrePtr = dependencyNodeCurrPtr;
        dependencyNodeCurrPtr = dependencyNodeCurrPtr->getParentNode();
    }

    // The total order is established as follows,
    // The parent node is the smallest, this case is captured above
    // The following block focuses on the case where both LCA reps are children
    //
    // Older children are tagged with lower sibling order, where newer ones are
    // tagged with higher sibling order, however, in algorithm, we use the traversal
    // order where we visit parent, newer children to older children, lowest to highest.
    // Thus, lower sibling order has higher priority.
    if (initiatorNodePrePtr->getSiblingOrder() < dependencyNodePrePtr->getSiblingOrder()) {
#ifdef LOG
    std::cout << "[PromiseLCA] Found a greater to lesser wait-for";
    std::cout << " " << initiatorNode->getNodeID() << " waits on " << dependencyNode->getNodeID() << std::endl;
#endif
        greaterNodeWaitingOnLesserNode(initiatorNodePrePtr, dependencyNodePrePtr, dependencyPromise);
    } else {
#ifdef LOG
        std::cout << "[PromiseLCA] Found a lesser to greater wait-for";
        std::cout << " " << initiatorNode->getNodeID() << " waits on " << dependencyNode->getNodeID() << std::endl;
#endif
        lesserNodeWaitingOnGreaterNode(initiatorNodePrePtr, dependencyNodePrePtr, dependencyPromise);
    }
}

} // namespace

//////////////////////////////////////////////////////////////////////////////
// Runtime API Implementation
//////////////////////////////////////////////////////////////////////////////
void verifyFutureWaitFor(TJFutureBase* dependencyFuture) {
#ifdef ENABLE_FUTURE_LCA
    if (dependencyFuture->isFulfilled()) {
        return;
    }
    bumpFutureLCACount();
    TaskNode* initiatorNode = getCurrentTaskNode();
    bool res = verifyFutureWaitForImp(initiatorNode, dependencyFuture);
    if (!res) throw std::runtime_error("Future LCA Verification failed, see log");
#endif // ENABLE_FUTURE_LCA
}

void verifyFutureWaitWithLCA(TaskNode* currTaskNode, TJFutureBase* dependencyFuture) {
#ifdef ENABLE_FUTURE_LCA
    if (dependencyFuture->isFulfilled()) {
        return;
    }
    bumpFutureLCACount();
    bool res = verifyFutureWaitForImp(currTaskNode, dependencyFuture);
    if (!res) throw std::runtime_error("Future LCA Verification failed, see log");
#endif // ENABLE_FUTURE_LCA
}

void verifyPromiseWaitFor(TJPromiseBase* dependencyPromise) {
#ifdef ENABLE_PROMISE_LCA
    if (dependencyPromise->isFulfilled()) {
        // If a promise is fulfilled already, it is safe from LCA inspection
        return;
    }
    bumpPromiseLCACount();
    TaskNode* initiatorNode = getCurrentTaskNode();
    verifyPromiseWaitForImp(initiatorNode, dependencyPromise);
#endif // ENABLE_PROMISE_LCA
}

void transferPromiseOwnership(TJPromiseBase* promiseToTransfer, TaskNode* newTaskNode) {
    auto currentNode = getCurrentTaskNode();
    if (!newTaskNode || !currentNode) {
        return;
    }

    if (promiseToTransfer->getOwnerTaskNode() != currentNode) {
        throw std::runtime_error("Current TaskNode does not own this promise");
    }

    promiseToTransfer->getOwnerTaskNode()->removeTJPromise(promiseToTransfer);
    promiseToTransfer->setNewOwnerTaskNode(newTaskNode);
    newTaskNode->addNewTJPromise(promiseToTransfer);
}

//////////////////////////////////////////////////////////////////////////////
// TJPromise Fulfillment Scope Guard Implementation
//////////////////////////////////////////////////////////////////////////////
TJPromiseFulfillmentScopeGuard::TJPromiseFulfillmentScopeGuard(TaskNode* currentScopeTaskNode)
    : currentScopeTaskNode_(currentScopeTaskNode) {}

TJPromiseFulfillmentScopeGuard::~TJPromiseFulfillmentScopeGuard() noexcept(false) {
#ifdef ENABLE_PROMISE_FULFILLMENT_CHECK
#ifdef LOG
    std::cout << "[Runtime API] Checking promises owned by ";
    std::cout << currentScopeTaskNode_->getNodeID() << std::endl;
#endif // LOG

    for (auto* promise : getCurrentTaskNode()->ownedPromises) {
        if (!promise->isFulfilled()) {
#ifdef LOG
            std::cout << "[Runtime API] A TJPromise is not fulfilled in node ";
            std::cout << currentScopeTaskNode_->getNodeID() << std::endl;
#endif // LOG
            // An exception is not used here since it is not catchable
            assert(false && "A TJPromise is not fulfilled in this task");
        }
    }

#ifdef LOG
    std::cout << "[Runtime API] All owned TJPromises are fulfilled in node ";
    std::cout << currentScopeTaskNode_->getNodeID() << std::endl;
#endif // LOG

#endif // ENABLE_PROMISE_FULFILLMENT_CHECK
}

/*
 * Stats API Implementation
 */
void bumpTaskNodeCount() {
    numNodes++;
}

void bumpFutureLCACount() {
    numFutureLCA++;
}

void bumpPromiseLCACount() {
    numPromiseLCA++;
}

void printStats() {
    std::cout << "[Transitive Joins] Task Nodes Created " << numNodes << std::endl;
    std::cout << "[Transitive Joins] Future LCA Invoked " << numFutureLCA << std::endl;
    std::cout << "[Transitive Joins] Promise LCA Invoked " << numPromiseLCA << std::endl;
}

} // namespace transitivejoins
} // namespace hclib