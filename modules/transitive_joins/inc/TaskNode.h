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

//////////////////////////////////////////////////////////////////////////////
// TaskNode Class Declarations
//////////////////////////////////////////////////////////////////////////////
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

    // Counter to track promise LCA
    // Three cases:
    // > 0: waiting on lesser nodes
    // = 0: no edges currently
    // < 0: waited by lesser nodes
    std::atomic<int32_t> edgeCount{0};

#ifdef LOG
    uint32_t getNodeID();
#endif

private:
    TaskNode* parentTaskNode_ = nullptr;
    // The nth spawned task from parent task node
    uint32_t siblingOrder_ = 0;

    // Number of children, also used to generate
    std::atomic_uint32_t numChildren_{0};

    // depth of this node
    uint32_t depth_ = 0;

    // Stores all owned promises in a set
    std::unordered_set<TJPromiseBase*> ownedPromises;

    // A scope guard struct to perform the actual TJPrmise Fulfillment
    friend struct TJPromiseFulfillmentScopeGuard;

#ifdef LOG
    // Unique Node ID for debugging
    uint32_t nodeID_ = 0;

    // Keep track of worker thread
    uint32_t workerThreadID_ = 0;
#endif

};

} // namespace transitivejoins
} // namespace hclib