#include "tests/TestFramework.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string filter = "";
    if (argc > 1) {
        filter = argv[1];
    }
    return test::TestRunner::runAll(std::cout, filter);
}
