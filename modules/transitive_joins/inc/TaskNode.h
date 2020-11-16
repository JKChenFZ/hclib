#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <unordered_set>

namespace hclib {
namespace transitivejoins {
// Forward Declaration
class TJPromiseBase;
struct TJPromiseFulfillmentScopeGuard;

/*
 * Representation of a Task during the runtime
 */
struct TaskNode {
public:
    TaskNode();
    TaskNode(TaskNode* parentNode);
    ~TaskNode();

    TaskNode* getParentNode();
    uint32_t getSiblingOrder();
    uint32_t getNumChildren();
    uint32_t getSiblingOrderForNewChild();
    uint32_t getDepth();

    // Operations related to adding/removing owner promises
    void removeTJPromise(TJPromiseBase* promiseToRemove);
    void addNewTJPromise(TJPromiseBase* newPromise);

    /* 
     * Counter to track promise LCA
     * Three cases:
     * > 0: waiting on lesser nodes
     * = 0: no edges currently
     * < 0: waited by lesser nodes
     */
    std::atomic<int32_t> edgeCount{0};

    // Returns "0" if LOG flag is not enabled
    uint32_t getNodeID();

private:
    TaskNode* parentTaskNode_ = nullptr;

    // The nth spawned task from parent task node
    uint32_t siblingOrder_{0};

    // Number of children, also used to generate the sibling order of new children
    std::atomic_uint32_t numChildren_{0};

    // Depth of this node in the task tree
    uint32_t depth_{0};

    // Stores all owned promises in a set
    std::unordered_set<TJPromiseBase*> ownedPromises;

    // A scope guard used to perform owned promises Fulfillment check
    friend struct TJPromiseFulfillmentScopeGuard;

    // Unique Node ID for debugging
    uint32_t nodeID_ = 0;
};

/*
 * Retrieve the current task node with respect to the
 * executing task. Creates a new one if one not found.
 * 
 * Returns "null" if ENABLE_TASK_TREE is not set
 */
TaskNode* getCurrentTaskNode();

/*
 * Sets the TaskNode of the current task
 * to the given parameter (can be nullptr)
 */
void setUpTaskNode(TaskNode* currTaskNode);

/*
 * Allocate a new TaskNode with the current TaskNode
 * as the parent
 * "null" if ENABLE_TASK_TREE is not set
 */
TaskNode* generateNewTaskNode();

} // namespace transitivejoins
} // namespace hclib