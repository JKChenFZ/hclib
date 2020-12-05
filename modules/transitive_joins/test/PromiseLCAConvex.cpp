#include <chrono>
#include <cstdint>
#include <iostream>
#include <stdlib.h>
#include <thread>

#include "hclib_cpp.h"

#include "TJAsyncAPI.h"

/*
    Implementation of a convex turn as specified in the paper
*/

namespace tj = hclib::transitivejoins;
int main(int argc, char** argv) {
    const char* deps[] = { "system", "transitive_joins" };
    hclib::launch(deps, 2, [&]() {
        hclib::finish([]() {
            hclib::async([]() {
                tj::TJPromise<int>* promiseX = tj::getNewTJPromise<int>();
                tj::TJPromise<int>* promiseY = tj::getNewTJPromise<int>();
                std::vector<tj::TJPromiseBase*> firstPromiseToTransfer {promiseX};
                std::vector<tj::TJPromiseBase*> secondPromiseToTransfer {promiseY};
                
                tj::async(secondPromiseToTransfer, [promiseY]() {
                    std::this_thread::sleep_for(std::chrono::seconds(3));
                    assert(tj::getCurrentTaskNode() == promiseY->getOwnerTaskNode());
                    promiseY->put(1);
                });

                auto handle = tj::async_future([promiseX]() {
                    assert(tj::getCurrentTaskNode() != promiseX->getOwnerTaskNode());
                    assert(promiseX->wait() == 10);

                    // Convex turn passes LCA
                    std::cout << "Pass Test" << std::endl;

                    return 0;
                });

                assert(promiseY->wait() == 1);
                promiseX->put(10);

                // Hclib has a runtime bug where the inner task can finish after the outer
                // task but the finish scope won't terminate
                //
                // To address this issue, the newer child returns a handle and the outer
                // task only terminates when the new child is able to finish the convex turn 
                handle->wait();
            });
        });
    });
    return 0;
}