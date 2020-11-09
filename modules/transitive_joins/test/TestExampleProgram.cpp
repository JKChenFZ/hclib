#include "AsyncAPI.h"
#include "TJPromise.h"

#include <iostream>

namespace tj = hclib::transitivejoins;

int main(int argc, char * argv[]) {
    const char *deps[] = { "system", "transitive_joins" };
    hclib::launch(deps, 2, [=] {
        auto promise = tj::getNewTJPromise<int>();
        promise->put(1);
        std::cout << "Worked " << promise->get_future()->get() << std::endl;
    });
}