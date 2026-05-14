#include <iostream>

#include "stm_test_common.h"

int main() {
    std::cout << "Running UNIT tests...\n";
    test_basic_push_pop();
    test_insert_delete_at();
    test_copy_move_equality();
    test_insert_delete_exceptions();
    std::cout << "UNIT tests passed.\n";
    return 0;
}
