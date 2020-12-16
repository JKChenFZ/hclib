# Transitive Joins
## Overview
- This folder is the implementation of the `Transitive Joins(TJ)` module, a set of runtime deadlock detection algorithms.
- The implementation is based on [Dynamic Deadlock Avoidance](https://calebvoss.com/research.html) by [Caleb Voss](https://calebvoss.com/)
- As compared to existing Hclib APIs, TJ APIs are merely wrappers of existing APIs; Hclib programs can be converted to use TJ with a minimal amount of code changes.
- All New APIs/features are available under the `hclib::transitivejoins` namespace

## Feature: Runtime Task Tree
- Each created Hclib Task (e.g. via `async(lambda)`) is represented by a node (`TaskNode`) with a parent pointer. The parent field points to the TaskNode of the Hclib Task which spawns the child task.
- The task tree is a foundation building block of the remaining features. High-level features detect deadlocks via LCA (lowest common ancestor) projection on the task tree.
- TJ implements this feature by wrapping existing task APIs (`async(lambda)` and `async_future(lambda)` as of right now) to inject necessary logic

## Feature: Future Deadlock Detection
- Each Future object (`TJFuture`) in Transitive-Joins has an owner TaskNode. The TaskNode is automatically assigned to Future object on creation based on the corresponding Hclib task.
- The owner does not change throughout the Future object life cycle.
- When the `wait` function on a future object is invoked, the node representing the executing task and the node assigned to the future object will go through the LCA projection and verification process.
- If the LCA representatives (the nodes directly below the LCA node) of two nodes above are in the right order (the one invoking the `wait` function is newer), the wait-for relationship is deemed correct. An exception will be thrown if the wait-for relationship is incorrect.
- An example of the LCA representative nodes are shown [here](https://docs.google.com/document/d/1Wr3b5VMaFJYOwXKpJ3AsZD2kdjeC-D2hhs2BpRilkuI/edit?usp=sharing)
- This module introduces `TJFuture` as a wrapper to the hclib `future` to inject the verification logic.

## Feature: Promise Ownership and Fulfillment Tracking
- Each Promise object also has an owner node. The node is initially the owner node of the task creating the said promise.
- The owner task (based on the owner node) must fulfill the promise in order to avoid deadlock in the host program.
- By the end of a Hclib task execution, a verification function will check whether all of the owned promises of the current task are fulfilled. An exception will be thrown if one is not fulfilled. TJ implements this by associating Promises with the corresponding TaskNode.
- The ownership can only be transferred, but it can only be transferred to the immediate children explicitly.
- In the module, `TJPromise` is introduced as a wrapper of the hclib `promise` to inject the ownership logic.
- `TJPromise` can only be created via the function `hclib::transitivejoins::getNewTJPromise` ([source](https://github.com/JKChenFZ/hclib/blob/bdf7bf8d26d063d157ebd761f4cfcd91b920c464/modules/transitive_joins/inc/TJPromise.h#L20)); this function ensures that the created `TJPromise` is on the heap with the correct owner TaskNode.
- The wrapper Task APIs can take an additional parameter (a vector of promises, [source](https://github.com/JKChenFZ/hclib/blob/bdf7bf8d26d063d157ebd761f4cfcd91b920c464/modules/transitive_joins/inc/TJAsyncAPI.h#L91)) to transfer promise ownership while creating a new task. See the snippet below for an example
```
const char *deps[] = { "system", "transitive_joins" };
hclib::launch(deps, 2, [=] {
    hclib::finish([&]() {
        auto firstPromise = transitivejoins::getNewTJPromise<int>();
        auto secondPromise = transitivejoins::getNewTJPromise<int>();
        
        std::vector<tj::TJPromiseBase*> promisesToPass;
        promisesToPass.push_back(firstPromise);
        promisesToPass.push_back(secondPromise);
        
        transitivejoins::async(promisesToPass, [firstPromise, secondPromise]() {
            firstPromise->put(1);
            secondPromise->put(2);
        });
    });
}
```

## Feature: Promise Deadlock Verification
- Since Promises can have dynamic ownerships, Promise deadlock verification detects cycles by monitoring the edges formed by the `wait` function invocations.
- The edges are always formed at the LCA representative nodes (defined the same way as above). Instead of directly noting the edges, a Promise uses an atomic counter to track lesser and greater edges.
- A total order is defined among nodes of the same depth under the same parent. The parent is the smallest and the oldest children tasks is the greatest.
- Lesser edge is formed when a `wait-for` relationship is detected from a lesser node to a greater node. The greater node acknowledges this edge by decrementing the counter on the greater node.
- Greater edge is formed in the reversed way. The counter on the greater node is incremented.
- If the counter is ever positive after a decrement operation(or negative after an increment operation), a cycle is detected and an exception is thrown.
- When the `put` function is called on a promise, previous increment/decrement operations will be undone. A fulfilled Promise is always exempt from the verification process.
- It is important to note that this approach is only a weak cycle dectection; a concave turn (defined in the paper), where the oldest child task is being waited on by both the parent and a newer task is banned even though no cycle is formed. 
- To differentiate future verification from promise verification explicitly, `TJPromise` in the module can be waited; the `wait` function on `TJPromise` will trigger promise verification and the `wait` function on `TJFuture` will trigger future verification.

## Example Usage
- The example below shows how the parent task can wait on a child task with promise verification
- Additional code snippets can be found under the `test` folder
```
const char* deps[] = { "system", "transitive_joins" };
hclib::launch(deps, 2, [&]() {
    hclib::finish([]() {
        auto promiseX = tj::getNewTJPromise<int>();
        std::vector<tj::TJPromiseBase*> promiseToTransfer {promiseX};
            
        // Promise is transferred through the first parameter, it must also be specified in the lambda capture
        // list so that it can be used within the lambda
        tj::async_future(promiseToTransfer, [promiseX]() {
            auto result = performComputation();
            promiseX->put(result);
        });

        // LCA will be projected and the wait-for will be verified
        promiseX->wait();
    });
});
```

## Builds
- Different builds are available to illustrate different features
- Runtime Task Tree
    - `make withTaskNodes`
    - Constructs the runtime Task Tree
- LCA Verifications
    - `make withLCA`
    - Runs Future LCA and Promise LCA algorithms
    - Also includes the runtime task tree (this is a required dependency)
- Everything
    - `make withAll`
    - Runtime Task Tree
    - Future and Promise LCA Algorithms
    - Promise Fulfillment Check
- `make install` is needed to place the shared library at the correct location
- __Without any flags, all the wrappers remain functional without any TJ features__

## Code Pointers
- `TJFuture.h`
    - A wrapper of the hclib future with verification
- `TJPromise.h`
    - A wrapper of the hclib promise with verification
- `TaskNode.h`
    - Representation of a node associated with a Hclib task
- `VerificationAPI.h`
    - Includes the concrete implementations of the verification algorithms and various ultility functions
- `TJAsyncAPI.h`
    - Wrapper of a subset of Hclib APIs with `TJPromise` and `TJFuture` integrated
    - This file should be included to any files wanting to use the `Transitive Joins` modules
    - Example usages can be found under the `test` directory

## Limitations
- This module implementation modifies the core implementation.
    - A task local (`void**`) is being added to `hclib_task_t` under `inc/hclib-task.h`
    - A `hclib_get_curr_task_local` function is added to retrieve the previous task local pointer

## Tests and Performance Evaluation 
- Tests are available under the `test` directory
- Performance Analysis is available under the `benchmarks` directory
