#pragma once

#include "hclib_cpp.h"

namespace hclib {
namespace transitivejoins {

// Forward declaration
struct TJPromiseBase;
void verifyPromiseWaitFor(TJPromiseBase* dependencyPromise);

/*
 * Wrapper of future_t, a part of TJPromise
 */
template<typename T>
struct TJPromFuture {
    T wait() {
#ifdef ENABLE_PROMISE_LCA
        verifyPromiseWaitFor(parentTJPromise_);
#endif
        return hclibFuture_->wait();
    }

    TJPromiseBase* parentTJPromise_;
    hclib::future_t<T>* hclibFuture_;
};

} // namespace transitivejoins
} // namespace hclib