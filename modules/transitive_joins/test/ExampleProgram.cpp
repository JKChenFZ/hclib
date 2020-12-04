#include "TJAsyncAPI.h"
#include "TJPromise.h"

#include <iostream>

namespace tj = hclib::transitivejoins;

int main(int argc, char * argv[]) {
    const char *deps[] = { "system", "transitive_joins" };
    hclib::launch(deps, 2, [=] {
        auto promise = tj::getNewTJPromise<int>();
        promise->put(1);
        assert(promise->get_future()->get() == 1 && "Incorrect promise value");
        std::cout << "Pass Test" << std::endl;
    });
}