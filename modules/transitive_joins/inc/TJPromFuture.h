#pragma once

#include "hclib_cpp.h"

namespace hclib {
namespace transitivejoins {

template<typename U>
struct TJPromise;

struct TJPromiseBase;
// Forward declaration of helper function in Verification API
void verifyPromiseWaitFor(TJPromiseBase* dependencyPromise);

//////////////////////////////////////////////////////////////////////////////
// TJPromFuture Declarations
// For futures constructed from TJPromise only
//////////////////////////////////////////////////////////////////////////////

// Specialized for general types
template<typename T>
struct TJPromFuture {
    T wait() {
        // No-op if ENABLE_Promise_LCA is not set
        verifyPromiseWaitFor(parentTJPromise_);

        return hclibFuture_->wait();
    }

    template<typename U>
    friend struct TJPromise;

private:
    TJPromise<T>* parentTJPromise_;
    hclib::future_t<T>* hclibFuture_;
};

} // namespace transitivejoins
} // namespace hclib