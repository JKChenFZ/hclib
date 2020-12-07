# Transitive Joins
## Overview
- This folder is the implementation of the `Transitive Joins` module, a set of runtime deadlock detection algorithms.
- The implementation is based on this [paper](https://calebvoss.com/assets/VCS19.pdf) by [Caleb Voss](https://calebvoss.com/)
- Specifically, following features are integrated with Hclib
    - Runtime Task Tree
        - Each created Hclib Task (e.g. `async()`) is represented by a node with a parent pointer. The parent field points to the node of the Hclib Task which spawns the child task.
    - Future LCA (Lowest Common Ancestor)
        - Each Future object has an owner node. The node is automatically assigned to the corresponding child task.
        - When the `wait` function on a future is invoked, the node representing the executing task and the node assigned to the future object will go through Future LCA verification.
        - If the LCA representatives of two nodes (the nodes directly below the LCA node) are in the right order (the one invoking the `wait` function is younger), the wait-for relationship is deemed correct.
    - Promise Fulfillment
        - Each Promise also has an owner node. The node is initially the owner node of the task creating the said promise.
        - The ownership can only be passed down the runtime node tree explicitly.
        - By the end of a Hclib task execution, a verification function will check whether all of the owned promises of the current tasks are fulfilled.
    - Promise LCA
        - Since Promises can have dynamic ownership, Promise LCA detects cycles by monitoring the edges formed by `wait` function invocations.
        - The edges are always formed at the LCA representative nodes. Instead of directly noting the edges, Promise LCA uses an atomic counter to track lesser and greater edges.
        - A total order is defined among nodes of the same depth under the same parent. The parent is the smallest and the oldest children tasks is the greatest.
        - Lesser edge is formed when a `wait-for` relationship is detected from a lesser node to a greater node. The greater node acknowledges this edge by decrementing the counter on the greater node.
        - Greater edge is formed in the reversed way. The counter on the greater node is incremented.
        - If the counter is ever positive after a decrement operation(or negative after an increment operation), a cycle is detected.
        - When the `put` function is called on a promise, previous increment/decrement operations will be reversed.
        

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