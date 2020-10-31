#include "AsyncAPI.h"

#include <iostream>

int main(int argc, char * argv[]) {
    const char *deps[] = { "system", "transitive_joins" };
    hclib::launch(deps, 2, [=] {
        std::cout << "Worked" << std::endl;
    });
}