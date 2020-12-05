#include <chrono>
#include <cstdint>
#include <iostream>
#include <stdlib.h>
#include <thread>

#include "hclib_cpp.h"

#include "TJAsyncAPI.h"

/*
    Implementation of a concave cycle between two children
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
                    try {
                        promiseY->wait();
                    } catch (std::runtime_error& e) {
                        // Depending on the execution order, either this or the
                        // one below will execute
                        assert(std::string(e.what()).compare("LCA Verification failed, see log") == 0);
                        std::cout << "Pass Test" << std::endl;
                        exit(1);
                    }
                });

                tj::async(secondPromiseToTransfer, [promiseX, promiseY]() {
                    assert(tj::getCurrentTaskNode() == promiseY->getOwnerTaskNode());
                    try {
                        promiseX->wait();
                    } catch (std::runtime_error& e) {
                        // Depending on the execution order, either this or the
                        // one above will execute
                        assert(std::string(e.what()).compare("LCA Verification failed, see log") == 0);
                        exit(1);
                    }
                });
            });
        });
    });
    return 0;
}