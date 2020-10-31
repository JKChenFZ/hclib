#pragma once

#include <atomic>
#include <mutex>
#include <vector>

#include "TaskNode.h"
#include "TJPromFuture.h"

#include "hclib_cpp.h"

namespace hclib {
namespace transitivejoins {
struct TJPromiseBase;

// Forward declaration of helper function in TJRuntimeAPI
void promiseLCASignalDependencyEdges(TJPromiseBase* promise);

// Restricting creation for TJPromises with heap only,
// since TJPromise returns a future as a pointer (meanwhile being a member object)
// it can be misleading that future is heap allocated.
template<typename U>
auto getNewTJPromise();

struct TJPromiseBase {
    virtual bool isFulfilled() = 0;
    void setNewOwner(TaskNode* newOwnerNode) { ownerTaskNode_ = newOwnerNode; }

    TaskNode* getOwnerTaskNode() { return ownerTaskNode_; }

    // These two functions are invoked from the Runtime API
    // to avoid adding flags in header files
    void addDependencyNode(TaskNode* newDependencyNode) {
        std::lock_guard<std::mutex> lock(dependencyNodesLock);
        dependencyNodes.push_back(newDependencyNode);
    }

    void signalAllDependencyNodes() {
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

protected:
    std::atomic<TaskNode*> ownerTaskNode_;
    // On the promise level, we remember the nodes for which we will signal
    // after "put" is called
    std::mutex dependencyNodesLock;
    std::vector<TaskNode*> dependencyNodes;

    TJPromiseBase() {}
};

// Specialized for general types
template<typename T>
struct TJPromise : TJPromiseBase {
    void put(T datum) {
        hclibPromise_.put(datum);
        // No-op if ENABLE_Promise_LCA is not set
        // ownerTaskNode_.load()->undoEdgesForPromise(this);
        promiseLCASignalDependencyEdges(this);
    }

    TJPromFuture<T>* getFuture() {
        return &tjPromFuture_;
    }

    bool isFulfilled() {
        return hclibPromise_.satisfied;
    }

    friend auto getNewTJPromise<T>();

private:
    hclib::promise_t<T> hclibPromise_;
    TJPromFuture<T> tjPromFuture_;

    TJPromise() {
        tjPromFuture_.hclibFuture_ = hclibPromise_.get_future();
        tjPromFuture_.parentTJPromise_ = this;
    }
};

// Specialized for void
template<>
struct TJPromise<void> : TJPromiseBase {
    void put() {
        hclibPromise_.put();
        // No-op if ENABLE_Promise_LCA is not set
        promiseLCASignalDependencyEdges(this);
    }

    TJPromFuture<void>* getFuture() {
        return &tjPromFuture_;
    }

    bool isFulfilled() {
        return hclibPromise_.satisfied;
    }

    friend auto getNewTJPromise<void>();

private:
    hclib::promise_t<void> hclibPromise_;
    TJPromFuture<void> tjPromFuture_;

    TJPromise() {
        tjPromFuture_.hclibFuture_ = hclibPromise_.get_future();
        tjPromFuture_.parentTJPromise_ = this;
    }
};

} // namespace transitivejoin
} // namespace hclib