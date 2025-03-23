#pragma once
#include "ShiftToMiddleArray.h"

void benchmark_parallel_deques(int n = 100, int ops_per_element = 1000);
void benchmark_parallel_shift_to_middle_array(int n = 100, int ops_per_element = 100);
