#pragma once

#include <iostream>
#include <atomic>

#include "hclib_cpp.h"

namespace hclib {
namespace transitivejoins {

// Forward declarations
struct TaskNode;
struct TJFutureBase;
void verifyFutureAwaitFor(TJFutureBase* dependencyFuture);

//////////////////////////////////////////////////////////////////////////////
// TJFuture Declarations
// For wrapping Hclib futures only
//////////////////////////////////////////////////////////////////////////////

struct TJFutureBase {
    virtual bool fulfilled() = 0;
    TaskNode* getAssociatedTaskNode() {
        return ownerTaskNode_;
    }

protected:
    TJFutureBase(TaskNode* ownerTaskNode) : ownerTaskNode_(ownerTaskNode) {}
    std::atomic<TaskNode*> ownerTaskNode_;
};

// Specialized for scalar types
template<typename T>
struct TJFuture : TJFutureBase {
    TJFuture(
        hclib::future_t<T>* hclibFuture,
        TaskNode* ownerTaskNode
    ) : TJFutureBase(ownerTaskNode), hclibFuture_(hclibFuture) {}

    T wait() {
        verifyFutureAwaitFor(this);

        return hclibFuture_->wait();
    }

    hclib::future_t<T>* getHclibFuture() {
        return hclibFuture_;
    }

    bool fulfilled() override {
        return hclibFuture_->owner->satisfied;
    }

private:
    hclib::future_t<T>* hclibFuture_;
};

} // namespace transitivejoins
} // namespace hclib