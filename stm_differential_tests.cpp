#include <iostream>

#include "stm_test_common.h"

int main() {
    std::cout << "Running DIFFERENTIAL tests (vs std::list)...\n";
    for (uint64_t seed = 1; seed <= 2; ++seed) {
        differential_randomized(seed, 200);
        std::cout << "Random differential progress: seed " << seed << "/2\n";
    }
    std::cout << "DIFFERENTIAL tests passed.\n";
    return 0;
}
