#pragma once

#include <atomic>
#include <mutex>
#include <vector>

#include "TaskNode.h"

#include "hclib_cpp.h"

namespace hclib {
namespace transitivejoins {
// Forward declaration
template<typename T>
struct TJPromise;
struct TJPromiseBase;
void verifyPromiseWaitFor(TJPromiseBase* dependencyPromise);

template<typename T>
auto getNewTJPromise() -> TJPromise<T>* {
    auto currentTaskNode = getCurrentTaskNode();
    auto newPromise = new TJPromise<T>(currentTaskNode);

    if (currentTaskNode) {
        // insert the new promise into the owner TaskNode
        currentTaskNode->addNewTJPromise(newPromise);
    }

    return newPromise;
}

/*
 * Common base interface of TJPromise
 */
struct TJPromiseBase {
    TJPromiseBase() = delete;
    TJPromiseBase(TaskNode* ownerTaskNode) : ownerTaskNode_(ownerTaskNode) {}

    TaskNode* getOwnerTaskNode();
    void setNewOwnerTaskNode(TaskNode* ownerTaskNode);
    void addDependencyNode(TaskNode* newDependencyNode);
    void signalAllDependencyNodes();

    virtual bool isFulfilled() = 0;

protected:
    std::atomic<TaskNode*> ownerTaskNode_;
    // On the promise level, we remember the nodes for which we will signal
    // after "put" is called
    std::mutex dependencyNodesLock;
    std::vector<TaskNode*> dependencyNodes;
};

// Specialized for general types
template<typename T>
struct TJPromise : public promise_t<T>, public TJPromiseBase {
    TJPromise() = delete;
    TJPromise(TaskNode* ownerTaskNode) : TJPromiseBase(ownerTaskNode) {}

    void put(T datum) {
        promise_t<T>::put(datum);
        signalAllDependencyNodes();
    }

    bool isFulfilled() override {
        return this->satisfied;
    }

    T wait() {
        verifyPromiseWaitFor(this);
        return promise_t<T>::get_future()->wait();
    }

    friend auto getNewTJPromise<T>() -> TJPromise<T>*;
};

// Specialized for void
template<>
struct TJPromise<void> : public promise_t<void>, public TJPromiseBase {
    TJPromise() = delete;
    TJPromise(TaskNode* ownerTaskNode) : TJPromiseBase(ownerTaskNode) {}

    void put() {
        promise_t<void>::put();
        signalAllDependencyNodes();
    }

    bool isFulfilled() override {
        return this->satisfied;
    }

    void wait() {
        verifyPromiseWaitFor(this);
        promise_t<void>::get_future()->wait();
    }

    friend auto getNewTJPromise<void>() -> TJPromise<void>*;
};

} // namespace transitivejoin
} // namespace hclib