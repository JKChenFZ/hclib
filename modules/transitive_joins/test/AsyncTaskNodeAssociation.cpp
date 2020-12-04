#include <atomic>
#include <cstdint>
#include <stdlib.h>
#include <iostream>

#include "TJAsyncAPI.h"

#include "hclib_cpp.h"

/*
Test construct the following runtime task tree
every line without dashes below represents a level in depth
                                            root
                                _____________________________
                                |                           |
                        firstTaskHandle             secondTaskHandle
                                                            -
                                                            |
                                                    firstNestedHandle
*/

namespace tj = hclib::transitivejoins;

int main(int argc, char** argv) {
    const char *deps[] = { "system", "transitive_joins" };
    hclib::launch(deps, 2, [=] {
        // First retrieve root node ID
        uint32_t rootNodeID = tj::getCurrentTaskNode()->getNodeID();

        hclib::finish([rootNodeID]() {
            // check the root node ID is preserved from "launch" to "finish"
            assert(rootNodeID == tj::getCurrentTaskNode()->getNodeID());

            auto firstTaskHandle = tj::async_future([&]() {
                // std::cout << "First logical task executing..." << std::endl;

                return tj::getCurrentTaskNode()->getNodeID();
            });

            auto secondTaskHandle = tj::async_future_await([&]() {
                // std::cout << "Second logical task executing..." << std::endl;

                // One Child tasks
                auto childTaskHandle = tj::async_future([&]() {
                    // std::cout << "Nested logical task executing..." << std::endl;

                    return tj::getCurrentTaskNode()->getNodeID();
                });

                uint32_t currentTaskNodeID = tj::getCurrentTaskNode()->getNodeID();
                // wait on completion of the child task before returning
                auto childTaskNode = childTaskHandle->getOwnerTaskNode();
                uint32_t childTaskReturnVal = childTaskHandle->wait();
                assert(childTaskReturnVal == childTaskNode->getNodeID());
                assert(currentTaskNodeID == childTaskNode->getParentNode()->getNodeID());

                return currentTaskNodeID;
            }, firstTaskHandle);

            // Wait for completion before test for values
            uint32_t firstTaskReturnVal = firstTaskHandle->wait();
            uint32_t secondTaskReturnVal = secondTaskHandle->wait();

            auto firstTaskNode = firstTaskHandle->getOwnerTaskNode();
            auto secondTaskNode = secondTaskHandle->getOwnerTaskNode();

            assert(firstTaskReturnVal == firstTaskNode->getNodeID());
            assert(secondTaskReturnVal == secondTaskNode->getNodeID());
            assert(firstTaskNode->getParentNode()->getNodeID() == rootNodeID);
            assert(secondTaskNode->getParentNode()->getNodeID() == rootNodeID);
        });
    });

    std::cout << "Pass Test" << std::endl;

    return 0;
}