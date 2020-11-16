#pragma once

#include "TaskNode.h"

#include "hclib_cpp.h"

namespace hclib {
namespace transitivejoins {
// Forward declaration
struct TJFutureBase;
void verifyFutureWaitFor(TJFutureBase* dependencyFuture);

struct TJFutureBase {
    TJFutureBase(TaskNode* ownerTaskNode) : ownerTaskNode_(ownerTaskNode) {}
    TaskNode* getOwnerTaskNode();

    virtual bool isFulfilled() = 0;

    TaskNode* ownerTaskNode_;
};

template<typename T>
struct TJFuture : public TJFutureBase {
    TJFuture(
        future_t<T>* hclibFuture,
        TaskNode* ownerTaskNode) : TJFutureBase(ownerTaskNode), hclibFuture_(hclibFuture) {}

    T wait() {
        verifyFutureWaitFor(this);
        return hclibFuture_->wait();
    }

    bool isFulfilled() override {
        return hclibFuture_->owner->satisfied;
    }

    future_t<T>* getHclibFuture() {
        return hclibFuture_;
    }

    future_t<T>* hclibFuture_;
};

template<>
struct TJFuture<void> : public TJFutureBase {
    TJFuture(
        future_t<void>* hclibFuture,
        TaskNode* ownerTaskNode) : TJFutureBase(ownerTaskNode), hclibFuture_(hclibFuture) {}

    void wait() {
        verifyFutureWaitFor(this);
        hclibFuture_->wait();
    }

    bool isFulfilled() override {
        return hclibFuture_->owner->satisfied;
    }

    future_t<void>* getHclibFuture() {
        return hclibFuture_;
    }

    future_t<void>* hclibFuture_;
};

} // namespace transitivejoins
} // namespace hclib