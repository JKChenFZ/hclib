# Transitive Joins Performance
## Overview
- This folder contains all the benchmarks used to evaluate the performance overhead of Transitive Joins
- Smith Waterman is based on the benchmark in the base module. [URL](https://github.com/habanero-rice/hclib/tree/master/test/smithwaterman)
- Mergesort is based on the code found online [URL](https://www.tutorialspoint.com/cplusplus-program-to-implement-merge-sort)

## Compilation and Execution
- Make sure that the `Transitive Joins` module is compiled and installed
- Use `make all` to compile
- A bash script is provided, `run.sh`, to easily start any benchmarks
- `./run.sh` will show all the accepted arguments

## Benchmarks
- Tile Smith Waterman With Promise LCA
    - The core algorithm stays the same, each step in the algorithm depends on three other promises. Overall runtime is `O(n * m)` where `n` is the length of the first sequence and `m` is the length of the second sequence.
    - Each Hclib task corresponds to a tile in the runtime matrix.
- Cell Smith Waterman with Promise LCA
    - Each Hclib task correspond to a cell in the runtime matrix.
    - The core algorithm is also modified such that each task depends on at most `n + m + ` other tasks (all the tasks in the same row and column plus the top left).
- Cell Smith Waterman with Future LCA
    - The algorithm is also modified as the one above. Instead of using promises, all promises are replaced by futures (As a result, Future LCA validation is triggered instead of Promise LCA validation)
- Mergesort Future LCA
    - An out-of-place implementation of the Mergesort sorting algorithm
    - When the array (sub-array) is being partitioned into the left and right half. A hclib task is spawned for each left and right recursive call. Both tasks are visible to the parent via future handles. The parent will wait on them before performing the merge step
- Mergesort Promise LCA
    - Based on the same implementation above. Futures are replaced by Promises.
    - Two promises (left and right) are created at each step, representing the left and right partition (hclib task), respectively.
    - The right Hclib task will wait on the left promise before executing. The parent will wait on the right promise before performing the merging step. The dependency chain is not ideal for concurrency; it is used to produce a complex LCA dependency scenario.

## Benchmark Configuration
- Executed on `Ubuntu 18 VM`
- The VM has `two CPU cores` with `12 GB` of memory
- The Host Machine is a Macbook Pro with `2.8GHz processor`
- Each benchmark is executed against `four` builds of `Transitive Joins` with `2` workers
    - Baseline (`make all`)
    - With Runtime Task Tree (`make withTaskNodes`)
    - With everything above and Future/Promise LCA Validations (`make withLCA`)
    - With everything above and Promise Fulfillment validation (`make withAll`)

## Runtime Stats
- The following charts shows the configuration used for each benchmark

| Benchmark Name | Configuration |
| - | - |
| Tile Smith Waterman With Promise LCA | `256x256` Tiles. `725x750` Total Tiles |
| Cell Smith Waterman with Promise LCA | `124x8004` Matrix |
| Cell Smith Waterman with Future LCA | `124x8004` Matrix |
| Mergesort Future LCA | `200,000,000` numbers, `1000` minimum partition size |
| Mergesort Promise LCA | `200,000,000` numbers, `1000` minimum partition size |

- The following chart shows the runtime stats for each benchmark. The numbers are consistent (if applicable) over different builds. The numbers of LCA verifications are averaged since there are two workers

| Benchmark Name | Total Task Nodes | Number of Future LCA Verifcations | Number of Promise LCA Verifications |
| - | - | - | - |
| Tile Smith Waterman With Promise LCA | `543,750` | `0` | `528,753` |
| Cell Smith Waterman with Promise LCA | `1,000,625` | `8068` (for dummy cells, not counted in execution time) | `1,825,116` |
| Cell Smith Waterman with Future LCA | `992,497` | `1,828,583` | N/A |
| Mergesort Future LCA | `524,287` | `262,144` | N/A |
| Mergesort Promise LCA | `524,287` | `1` (dummy) | `523,400` |


## Performance
- The following chart shows the performane in execution time

| Benchmark Name | Baseline | With Task Nodes | With LCA | With All |
| - | - | - | - | - |
| Tile Smith Waterman With Promise LCA | `27.896` | `27.996` (`1.0036x`) | `28.202` (`1.01x`) | `29.488` (`1.05x`) |
| Cell Smith Waterman with Promise LCA | `17.6122` | `17.47` (`.992x`) | `25.156` (`1.428x`) `#A` | `28.1014` (`1.59x`) |
| Cell Smith Waterman with Future LCA | `18.7914` | `22.3358` (`1.188x`) `#B` | `26.7124` (`1.42x`) `#A` | N/A |
| Mergesort Future LCA | `13.01` | `13.15` (`1.01x`) | `13.22` (`1.014x`) | N/A |
| Mergesort Promise LCA | `21.66` | `21.86` (`1.01x`) | `22.09` (`1.02x`) | `22.23` (`1.026x`) |

### Notes
1. For realistic benchmarks where each hclib task is responsible for non-trivial amount of computation, `Transitive Joins` adds about `5%` of the overhead. The `Fulfillment check` feature is the one with the heaviest performance impact.
2. For extreme benchmarks where each hclib task is responsible for trivial amount of computattion, `Transitive Joins` is expensive.
    - For runtime results tagged with `A`, the overhead can be divided into two parts. The first part is associated with the overwhelming number of future/promises. The second part is from the implementation detail of the verification functions. Every time a `wait` is called, the underlying verification will retrieve the `TaskNode` of the current executing hclib task. It depends on the `hclib` function `get_task_local`, which depends on the `pthread_getspecific` function. This is not cheap and the cost is ampifiled through the overwhelming number of invocations (at most `n + m + 1` at each cell/task).
    - For runtime result tagged with `B`, I observed from the `perf` tool that `malloc` is the main source of overhead. Besides allocating the `TaskNodes` for each task (about `1 million`), the algorithm also creates the `TJFutures` inline. For the same version with Promise LCA, the `TJPromises` were created ahead of the time.