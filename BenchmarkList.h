#include <vector>
#include <list>
#include <random>
#include <chrono>
#include <iostream>
#include <fstream>
#include "ShiftToMiddleArray.h"

using namespace std;

template <typename ContainerType>
double benchmark_random_operations(int size, int operations, const int iterations = 10) {
    std::mt19937 rng(42); // Fixed seed for reproducibility
    std::discrete_distribution<int> op_dist({30, 30, 30, 10}); // 10% chance for spike

    ContainerType container;
    bool spikeMode = false;
    volatile int stored_value = 0; // Prevent compiler optimizations

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        // Initial insertions
        for (int j = 0; j < size; ++j) {
            container.push_back(j);
        }

        // Mixed random operations
        for (int j = 0; j < operations; ++j) {
            if (container.empty()) continue;
            int index = rng() % container.size();

            int op = op_dist(rng);
            switch (op) {
                case 0: // Insert at random position
                    if (index < container.size()) container[index] = j;
                    else container.push_back(j);
                    break;
                case 1: // Remove if not empty
                    if (index < container.size()) container[index] = container.back(), container.pop_back();
                    break;
                case 2: // Read element
                    if (index < container.size()) stored_value = container[index];
                    break;
                case 3: // Spike event: randomly remove/add 10% of elements
                    int spike_size = container.size() / 10;
                    for (int k = 0; k < spike_size; ++k) {
                        int spike_index = rng() % container.size();

                        if (spikeMode && !container.empty()) {
                            container[spike_index] = container.back();
                            container.pop_back();
                        } else {
                            if (spike_index < container.size()) container[spike_index] = k;
                            else container.push_back(k);
                        }
                    }
                    spikeMode = !spikeMode; // Alternate spike behavior
                    break;
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    double total_time = std::chrono::duration<double, std::milli>(end - start).count();
    return total_time;
}

double benchmark_random_operations_list(int size, int operations, const int iterations = 10);
void run_benchmarks_list(int operations = 40000);
