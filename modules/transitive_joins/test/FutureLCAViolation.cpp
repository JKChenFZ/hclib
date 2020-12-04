#include <chrono>
#include <iostream>
#include <stdlib.h>
#include <thread>
#include <vector>

#include "hclib_cpp.h"

#include "TJAsyncAPI.h"

/*
Test construct the following runtime task tree
every line without dashes below represents a level in depth
                                            root
                                _____________________________
                                |                           |
                        (un-named first task)          (un-named second task)
LCA will trigger and throw when (un-named first task) invoke "wait" on secondNestedTaskHandle
*/

namespace tj = hclib::transitivejoins;

int main(int argc, char** argv) {
    const char* deps[] = { "system", "transitive_joins" };
    hclib::launch(deps, 2, [&]() {
        hclib::finish([]() {
            // I am using a variable here to transfer knowledge of the second handler
            // This variable has to be on heap, stack variable gets de-allocated after
            // the second task is scheduled. Used a vector because otherwise I would
            // need a double pointer
            auto handle = new std::vector<tj::TJFuture<int>*>();
            tj::async([handle]() {
                std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(3000));
                try {
                    // This line will raise error since the newer task will never finish
                    handle->front()->wait();
                    exit(1);
                } catch (std::runtime_error& e) {
                    std::cout << "Pass Test" << std::endl;
                    exit(1);
                }
            });

            handle->push_back(tj::async_future([]() {
                // This task will always be on spinning wait so that when the old tasks
                // waits on the newer task, an exception will be expected
                while (true) {
                    hclib::yield();
                }

                return 0;
            }));
        });
    });


    return 0;
}