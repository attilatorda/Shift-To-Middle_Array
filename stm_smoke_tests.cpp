#include <iostream>

#include "stm_test_common.h"

int main() {
    std::cout << "Running SMOKE tests...\n";
    test_basic_push_pop();
    test_serialize_roundtrip_small();
    std::cout << "SMOKE tests passed.\n";
    return 0;
}
