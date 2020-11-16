#pragma once

#include <vector>

#include "TaskNode.h"
#include "TJFuture.h"
#include "TJPromise.h"
#include "VerificationAPI.h"

#include "hclib_cpp.h"

namespace hclib {
namespace transitivejoins {
namespace {
// Wrap user lambda within another lambda to inject bookkeeping
// logic. E.g. set up TaskNode for the new hclib Async Task
#define inlineUserVoidLambdaSetUp(userLambda, newTaskNode)          \
[userLambda, newTaskNode]() {                                       \
    setUpTaskNode(newTaskNode);                                     \
    TJPromiseFulfillmentScopeGuard scopeGuard(newTaskNode);         \
                                                                    \
    userLambda();                                                   \
}

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

    hclib::async(inlineUserVoidLambdaSetUp(lambda, newTaskNode));
}

template <typename T, typename U>
void async_await(T&& lambda, TJFuture<U>* tjFuture) {
    TaskNode* newTaskNode = generateNewTaskNode();
    verifyFutureAwaitWithLCA(newTaskNode, tjFuture);

    hclib::async_await(
        inlineUserVoidLambdaSetUp(lambda, newTaskNode),
        tjFuture->getHclibFuture()
    );
}

template <typename T>
auto async_future(T&& lambda) -> TJFuture<decltype(lambda())>* {
    TaskNode* newTaskNode = generateNewTaskNode();

    auto hclibFuture = hclib::async_future(
        inlineUserLambdaSetUp(lambda, newTaskNode)
    );

    auto tjFuture = new TJFuture<decltype(lambda())>(
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
void async(std::vector<TJPromise<T>*> promisesToTransfer, U&& lambda) {
    TaskNode* newTaskNode = generateNewTaskNode();

    for (auto promise : promisesToTransfer) {
        // Change in ownership logic
        transferPromiseOwnership(promise, newTaskNode);
    }
    
    hclib::async(inlineUserVoidLambdaSetUp(lambda, newTaskNode));
}

} // namespace transitivejoins
} // namespace hclib