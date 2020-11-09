#pragma once

#include <atomic>
#include <mutex>
#include <vector>

#include "TaskNode.h"
#include "TJPromFuture.h"

#include "hclib_cpp.h"

namespace hclib {
namespace transitivejoins {

template <typename T>
auto getNewTJPromise();

/*
 * Common base interface of TJPromise
 */
struct TJPromiseBase {
    virtual bool isFulfilled() = 0;

    void setNewOwner(TaskNode* newOwnerNode);
    TaskNode* getOwnerTaskNode();

    void addDependencyNode(TaskNode* newDependencyNode);
    void signalAllDependencyNodes();

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
struct TJPromise : public promise_t<T>, public TJPromiseBase {
    void put(T datum) {
        signalAllDependencyNodes();
        promise_t<T>::put(datum);
    }

    bool isFulfilled() override {
        return this->satisfied;
    }

    friend auto getNewTJPromise<T>();

private:
    TJPromise() {}
};

// Specialized for void
template<>
struct TJPromise<void> : public promise_t<void>, public TJPromiseBase {
    void put() {
        signalAllDependencyNodes();
        promise_t<void>::put();
    }

    bool isFulfilled() override {
        return this->satisfied;
    }

    friend auto getNewTJPromise<void>();

private:
    TJPromise() {}
};

template<class T>
auto getNewTJPromise() -> TJPromise<T> {
    auto newPromise = new TJPromise<T>();
    auto currentTaskNode = getCurrentTaskNode();
    newPromise->setNewOwner(currentTaskNode);

    if (currentTaskNode) {
        // insert the new promise into the owner TaskNode
        getCurrentTaskNode()->addNewTJPromise(newPromise);
    }

    return newPromise;
}

} // namespace transitivejoin
} // namespace hclib