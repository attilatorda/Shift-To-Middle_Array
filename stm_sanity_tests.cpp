#include <iostream>

#include "stm_test_common.h"

int main() {
    std::cout << "Running SANITY tests...\n";
    test_sanity_boundaries();
    std::cout << "SANITY tests passed.\n";
    return 0;
}
