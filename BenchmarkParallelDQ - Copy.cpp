#include <deque>
#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include <omp.h>
#include "BenchmarkParallelDQ.h"

void benchmark_parallel_deques(int n, int ops_per_element) {
    std::vector<std::deque<int>> deques(n);

    // Initialize RNG
      std::mt19937 rng(42); // Fixed seed for reproducibility

    // Fill each deque with 10 elements at the head
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 10; ++j) {
            deques[i].push_front(j);
        }
    }

    int num_threads = omp_get_max_threads(); // Get max available cores
    std::cout << "Running with " << num_threads << " threads.\n";

    auto start = std::chrono::high_resolution_clock::now();

    // Parallel processing
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < n; ++i) {
        std::mt19937 local_rng(42 + i); // Unique seed per thread to avoid contention
        std::discrete_distribution<int> local_op_dist({15, 15, 20, 20, 5, 5, 20});

        for (int j = 0; j < ops_per_element; ++j) {
            int op = local_op_dist(local_rng);

            if (op == 0) deques[i].push_front(j);  // Add 1 to head
            else if (op == 1) deques[i].push_back(j); // Add 1 to tail
            else if (op == 2 && !deques[i].empty()) deques[i].pop_front(); // Remove head
            else if (op == 3 && !deques[i].empty()) deques[i].pop_back(); // Remove tail
            else if (op == 4) { // Add 50 to head
                for (int k = 0; k < 50; ++k) deques[i].push_front(k);
            }
            else if (op == 5) { // Add 50 to tail
                for (int k = 0; k < 50; ++k) deques[i].push_back(k);
            }
            else if (op == 6) { //Read random
                int size = deques[i].size();
                if (size > 0) {
                    int x = deques[i][rand() % size];
                }
            }

            if (deques[i].size() > 100) {
                for (int k = 0; k < 95; ++k) deques[i].pop_front();
            } else if (deques[i].empty()) {
                deques[i].push_front(0);
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    double total_time = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << "Parallel deque benchmark took " << total_time << " ms.\n";
}

void benchmark_parallel_shift_to_middle_array(int n, int ops_per_element) {
    std::vector<ShiftToMiddleArray<int>> arrays(n);

    // Initialize RNG
    std::mt19937 rng(42); // Fixed seed for reproducibility

    // Fill each array with 10 elements at the head
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 10; ++j) {
            arrays[i].insert_head(j);
        }
    }

    int num_threads = omp_get_max_threads(); // Get max available cores
    std::cout << "Running with " << num_threads << " threads.\n";

    auto start = std::chrono::high_resolution_clock::now();

    // Parallel processing
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < n; ++i) {
        std::mt19937 local_rng(42 + i); // Unique seed per thread to avoid contention
        std::discrete_distribution<int> local_op_dist({15, 15, 20, 20, 5, 5, 20});

        for (int j = 0; j < ops_per_element; ++j) {
            int op = local_op_dist(local_rng);

            if (op == 0) arrays[i].insert_head(j);  // Add 1 to head
            else if (op == 1) arrays[i].insert_tail(j); // Add 1 to tail
            else if (op == 2) arrays[i].remove_head(); // Remove head
            else if (op == 3) arrays[i].remove_tail(); // Remove tail
            else if (op == 4) { // Add 50 to head
                for (int k = 0; k < 50; ++k) arrays[i].insert_head(k);
            }
            else if (op == 5) { // Add 50 to tail
                for (int k = 0; k < 50; ++k) arrays[i].insert_tail(k);
            }
            else if (op == 6) { //Read random
                int size = arrays[i].size();
                if (size > 0) {
                    int x = arrays[i][rand() % size];
                }
            }

            if (arrays[i].size() > 100) {
                for (int k = 0; k < 95; ++k) arrays[i].remove_head();
            } else if (arrays[i].is_empty()) {
                arrays[i].insert_head(0);
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    double total_time = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << "Parallel ShiftToMiddleArray benchmark took " << total_time << " ms.\n";
}
