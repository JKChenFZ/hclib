
#include <assert.h>
#include <iostream>
#include <stdlib.h>

#include "TJAsyncAPI.h"

#include "hclib_cpp.h"

namespace tj = hclib::transitivejoins;

constexpr size_t NUM_TASKS = 1000;

int main (int argc, char ** argv) {
    const char *deps[] = { "system", "transitive_joins" };
    hclib::launch(deps, 2, [=] {
        hclib::finish([]() {
            for (size_t index = 0; index < NUM_TASKS; index++) {
                auto handle = tj::async_future([](){ return 1; });

                // Valid Parent waits for child relationship for future LCA
                assert(handle->wait() == 1);
            }
        });
    });

    std::cout << "Pass Test" << std::endl;

    return 0;
}