#include <chrono>
#include <cstdint>
#include <iostream>
#include <stdlib.h>
#include <thread>

#include "hclib_cpp.h"

#include "TJAsyncAPI.h"

/*
Adopted the following from Java Implementation
private void asyncTest ()
{
    Condition a = new Condition(); a.setFinalOwner();
    Condition b = new Condition();
    async(b).submit(() -> {
            b.signal();
            a.await();
        });
    b.await();
    a.signal();
}
*/

namespace tj = hclib::transitivejoins;
int main(int argc, char** argv) {
    const char* deps[] = { "system", "transitive_joins" };
    hclib::launch(deps, 2, [&]() {
        hclib::finish([]() {
            tj::TJPromise<int>* promiseX = tj::getNewTJPromise<int>();
            tj::TJPromise<int>* promiseY = tj::getNewTJPromise<int>();
            std::vector<tj::TJPromiseBase*> promiseToTransfer {promiseY};
            
            auto handle = tj::async_future(promiseToTransfer, [promiseX, promiseY]() {
                // Delays the fulfillment such that proper LCA is formed
                std::this_thread::sleep_for(std::chrono::seconds(3));
                promiseY->put(1);

                promiseX->wait();
                return 1;
            });

            promiseY->wait();

            // Delays the fulfillment such that proper LCA is formed
            std::this_thread::sleep_for(std::chrono::seconds(3));
            promiseX->put(1);

            // Due to a bug in the runtime, inner task must finish before the
            // termination of the outer task. Else, the program will never terminate
            handle->wait();
            std::cout << "Pass Test" << std::endl;
        });
    });
    return 0;
}