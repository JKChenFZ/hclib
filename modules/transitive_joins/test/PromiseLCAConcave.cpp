#include <chrono>
#include <cstdint>
#include <iostream>
#include <stdlib.h>
#include <thread>

#include "hclib_cpp.h"

#include "TJAsyncAPI.h"

/*
    Implementation of a concave turn as specified in the paper
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

                tj::async(firstPromiseToTransfer, [promiseX, promiseY]() {
                    assert(tj::getCurrentTaskNode() == promiseX->getOwnerTaskNode());
                    // Delays the wait, if wait too early, Y might not get passed over in time
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                    try {
                        // This task is being used to pass info between the parent
                        // and the new child, this is a concave turn since this task
                        // has the highest order
                        promiseY->wait();
                    } catch (std::runtime_error& e) {
                        // Depending on the execution order, either this or the
                        // one below will execute
                        assert(std::string(e.what()).compare("LCA Verification failed, see log") == 0);
                        std::cout << "Pass Test" << std::endl;
                        exit(1);
                    }

                    promiseX->put(10);
                });

                tj::async(secondPromiseToTransfer, [promiseY]() {
                    assert(tj::getCurrentTaskNode() == promiseY->getOwnerTaskNode());
                    // Stalls the fulfillment, if fulfilled, LCA will skip
                    std::this_thread::sleep_for(std::chrono::seconds(10));
                    promiseY->put(1);
                });

                assert(promiseX->wait() == 10);
            });
        });
        std::cout << "Finished and passed checkes, exiting..." << std::endl;
    });
    return 0;
}