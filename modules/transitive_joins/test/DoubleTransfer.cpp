#include <assert.h>
#include <chrono>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <thread>

#include "hclib_cpp.h"

#include "TJAsyncAPI.h"

namespace tj = hclib::transitivejoins;

int main(int argc, char** argv) {
    const char *deps[] = { "system", "transitive_joins" };
    hclib::launch(deps, 2, [=] {
        hclib::finish([&]() {
            tj::TJPromise<int>* promise = tj::getNewTJPromise<int>();
            std::vector<tj::TJPromiseBase*> promisesToPass;
            promisesToPass.push_back(promise);
            tj::async(promisesToPass, [promise]() {
                promise->put(1);
            });

            try {
                // This pass is invalid
                tj::async(promisesToPass, [promise]() {
                    promise->put(1);
                });
            } catch (std::runtime_error& e) {
                assert(std::string(e.what()).compare("Current TaskNode does not own this promise") == 0);
            }
        });
    });

    std::cout << "Pass Test" << std::endl;

    return 0;
}