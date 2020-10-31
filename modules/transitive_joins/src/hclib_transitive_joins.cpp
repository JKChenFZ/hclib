#include "hclib_transitive_joins.h"

#include <iostream>

HCLIB_MODULE_INITIALIZATION_FUNC(transitive_joins_pre_initialize) {
    std::cout << "[Transitive Joins] Initializing" << std::endl;
}

HCLIB_MODULE_INITIALIZATION_FUNC(transitive_joins_post_initialize) {
    std::cout << "[Transitive Joins] Initialized" << std::endl;
}

HCLIB_MODULE_INITIALIZATION_FUNC(transitive_joins_finalize) {
    std::cout << "[Transitive Joins] Ended" << std::endl;
}

HCLIB_REGISTER_MODULE("transitive_joins", transitive_joins_pre_initialize, transitive_joins_post_initialize, transitive_joins_finalize)