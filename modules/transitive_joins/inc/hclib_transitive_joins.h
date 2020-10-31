#pragma once

#include "hclib-module.h"
#include "hclib_cpp.h"

namespace hclib {

HCLIB_MODULE_INITIALIZATION_FUNC(transitive_joins_pre_initialize);
HCLIB_MODULE_INITIALIZATION_FUNC(transitive_joins_post_initialize);
HCLIB_MODULE_INITIALIZATION_FUNC(transitive_joins_finalize);

}; //namespace hclib