#pragma once
#include <array>
#include <deque>
#include <queue>
#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include <fstream>
#include <omp.h>
#include "ShiftToMiddleArray.h"
#include "ExpandingRingBuffer.h"

void run_benchmarks_queue(int operations);
