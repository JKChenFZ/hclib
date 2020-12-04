#include <assert.h>
#include <chrono>
#include <iostream>
#include <stdlib.h>
#include <thread>

#include "TJAsyncAPI.h"

#include "hclib_cpp.h"

namespace tj = hclib::transitivejoins;

int main(int argc, char** argv) {
    const char* deps[] = { "system", "transitive_joins" };

    hclib::launch(deps, 2, [&]() {
        hclib::finish([&]() {
            // Another task where a promise is created but not fulfilled
            // We can't use try-catch because the error happens in another thread
            tj::async([]() {
                tj::TJPromise<int>* _ = tj::getNewTJPromise<int>();
                std::this_thread::sleep_for(std::chrono::seconds(3));
            });

            // We are explicitly checking for assertion failure
        });
    });

    return 0;
}