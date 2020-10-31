#pragma once

#include "TJFuture.h"
#include "VerificationAPI.h"

#include "hclib_cpp.h"

namespace hclib {
namespace transitivejoins {

namespace {

// Capturing the lambda by reference can cause partial corruption in captured list
// most likely due to deallocation
#define inlineUserVoidLambdaSetUp(userLambda, newTaskNode)          \
[userLambda, newTaskNode]() {                                       \
    setUpTaskNode(newTaskNode);                                     \
    TJPromiseFulfillmentScopeGuard scopeGuard(newTaskNode);         \
                                                                    \
    userLambda();                                                   \
}

// Capturing the lambda by reference can cause partial corruption in captured list
// most likely due to deallocation
#define inlineUserLambdaSetUp(userLambda, newTaskNode)              \
[userLambda, newTaskNode]() {                                       \
    setUpTaskNode(newTaskNode);                                     \
    TJPromiseFulfillmentScopeGuard scopeGuard(newTaskNode);         \
                                                                    \
    return userLambda();                                            \
}

} // namespace

template <typename T>
void async(T&& lambda) {
    TaskNode* newTaskNode = generateNewTaskNode();

    hclib::async(
        inlineUserVoidLambdaSetUp(lambda, newTaskNode)
    );
}

template <typename T, typename U>
void async_await(
    T&& lambda,
    TJFuture<U>* tjFuture
) {
    TaskNode* newTaskNode = generateNewTaskNode();
    // No-op if ENABLE_FUTURE_LCA is not set
    verifyFutureAwaitWithLCA(newTaskNode, tjFuture);

    hclib::async_await(
        inlineUserVoidLambdaSetUp(lambda, newTaskNode),
        tjFuture->getHclibFuture()
    );
}

template <typename T>
auto async_future(T&& lambda) -> TJFuture<decltype(lambda())>* {
    TaskNode* newTaskNode = generateNewTaskNode();

    hclib::future_t<decltype(lambda())>* hclibFuture = hclib::async_future(
        inlineUserLambdaSetUp(lambda, newTaskNode)
    );

    TJFuture<decltype(lambda())>* tjFuture = new TJFuture<decltype(lambda())>(
        hclibFuture,
        newTaskNode
    );

    return tjFuture;
}

template <typename T, typename U>
auto async_future_await(
    T&& lambda,
    TJFuture<U>* tjFuture
) -> TJFuture<decltype(lambda())>* {
    TaskNode* newTaskNode = generateNewTaskNode();
    // No-op if ENABLE_FUTURE_LCA is not set
    verifyFutureAwaitWithLCA(newTaskNode, tjFuture);

    auto hclibFuture = hclib::async_future_await(
        inlineUserLambdaSetUp(lambda, newTaskNode),
        tjFuture->getHclibFuture()
    );

    TJFuture<decltype(lambda())>* newTJFuture = new TJFuture<decltype(lambda())>(
        hclibFuture,
        newTaskNode
    );
    return newTJFuture;
}

// APIs extended to accomendate transfer in promise ownership
template <typename T, typename U>
void async(TJPromise<T>* promiseToTransfer, U&& lambda) {
    TaskNode* newTaskNode = generateNewTaskNode();

    // Change in ownership logic
    transferPromiseOwnership(promiseToTransfer, newTaskNode);

    hclib::async(
        [promiseToTransfer, newTaskNode, lambda]() {
            setUpTaskNode(newTaskNode);
            TJPromiseFulfillmentScopeGuard scopeGuard(newTaskNode);

            lambda();
        }
    );
}

template <typename T, typename U>
auto async_future(TJPromise<T>* promiseToTransfer, U&& lambda) -> TJFuture<decltype(lambda())>* {
    TaskNode* newTaskNode = generateNewTaskNode();

    // Change in ownership logic
    transferPromiseOwnership(promiseToTransfer, newTaskNode);

    hclib::future_t<decltype(lambda())>* hclibFuture = hclib::async_future(
        [promiseToTransfer, newTaskNode, lambda]() {
            setUpTaskNode(newTaskNode);
            TJPromiseFulfillmentScopeGuard scopeGuard(newTaskNode);

            return lambda();
        }
    );

    TJFuture<decltype(lambda())>* tjFuture = new TJFuture<decltype(lambda())>(
        hclibFuture,
        newTaskNode
    );

    return tjFuture;
}

} // namespace transitivejoins
} // namespace hclib