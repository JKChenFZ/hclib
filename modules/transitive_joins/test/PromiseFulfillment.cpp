#include <assert.h>
#include <chrono>
#include <iostream>
#include <stdlib.h>
#include <thread>
#include <vector>

#include "TJAsyncAPI.h"

#include "hclib_cpp.h"

namespace tj = hclib::transitivejoins;

int main(int argc, char** argv) {
    const char* deps[] = { "system", "transitive_joins" };
    hclib::launch(deps, 2, [&]() {
        hclib::finish([&]() {
            // A task that creates and fulfill the promise
            // We don't expect an error here
            tj::async([]() {
                tj::TJPromise<int>* promise = tj::getNewTJPromise<int>();
                std::this_thread::sleep_for(std::chrono::seconds(3));

                promise->put(5);
            });

            // Test for ownership transfer, but the parent does not fulfill
            auto handle = tj::async_future([]() {
                tj::TJPromise<int>* promiseToPass = tj::getNewTJPromise<int>();
                std::vector<tj::TJPromiseBase*> promisesToTransfer;
                promisesToTransfer.push_back(promiseToPass);

                tj::async(promisesToTransfer, [promiseToPass]() {
                    // Verify that owner task node is changed
                    assert(promiseToPass->getOwnerTaskNode()->getNodeID() == tj::getCurrentTaskNode()->getNodeID());

                    promiseToPass->put(1);
                });
            });

            handle->wait();
        });
    });

    std::cout << "Pass Test" << std::endl;

    return 0;
}