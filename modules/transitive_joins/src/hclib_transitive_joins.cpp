#include "hclib_transitive_joins.h"

#include <iostream>

#include "VerificationAPI.h"

#define printFeatureModeStatus(name, variable) \
    std::cout << "[Transitive Joins] " << name <<  " is " << (variable ? "on" : "off") << std::endl;

HCLIB_MODULE_INITIALIZATION_FUNC(transitive_joins_pre_initialize) {
    std::cout << "[Transitive Joins] Initializing" << std::endl;
}

HCLIB_MODULE_INITIALIZATION_FUNC(transitive_joins_post_initialize) {
    bool taskTreeStatus = false;
    bool futureLCAStatus = false;
    bool promiseLCAStatus = false;
    bool promiseFulfillmentStatus = false;

#ifdef ENABLE_TASK_TREE
    taskTreeStatus = true;
#endif

#ifdef ENABLE_FUTURE_LCA
    futureLCAStatus = true;
#endif

#ifdef ENABLE_PROMISE_LCA
    promiseLCAStatus = true;
#endif

#ifdef ENABLE_PROMISE_FULFILLMENT_CHECK
    promiseFulfillmentStatus = true;
#endif
    printFeatureModeStatus("Task Tree", taskTreeStatus);
    printFeatureModeStatus("Future LCA", futureLCAStatus);
    printFeatureModeStatus("Promise LCA", promiseLCAStatus);
    printFeatureModeStatus("Promise Fulfillment", promiseFulfillmentStatus);
}

HCLIB_MODULE_INITIALIZATION_FUNC(transitive_joins_finalize) {
    std::cout << "[Transitive Joins] Ended" << std::endl;
    hclib::transitivejoins::printStats();
}

HCLIB_REGISTER_MODULE("transitive_joins", transitive_joins_pre_initialize, transitive_joins_post_initialize, transitive_joins_finalize)