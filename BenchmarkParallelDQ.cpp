#include <deque>
#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include <fstream>
#include <omp.h>
#include "BenchmarkParallelDQ.h"

using namespace std;

template <typename ContainerType>
double benchmark_parallel_containers(int n, int ops_per_element) {
    std::vector<ContainerType> containers(n);

    // Initialize RNG
    std::mt19937 rng(42); // Fixed seed for reproducibility

    // Fill each container with 10 elements at the head
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 10; ++j) {
            containers[i].push_front(j); // Assume push_front is implemented
        }
    }

    int num_threads = omp_get_max_threads(); // Get max available cores

    auto start = std::chrono::high_resolution_clock::now();

    // Parallel processing
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < n; ++i) {
        std::mt19937 local_rng(42 + i); // Unique seed per thread to avoid contention
        std::discrete_distribution<int> local_op_dist({15, 15, 20, 20, 5, 5, 20});

        for (int j = 0; j < ops_per_element; ++j) {
            int op = local_op_dist(local_rng);

            switch (op) {
                case 0: // Add 1 to head
                    containers[i].push_front(j);
                    break;
                case 1: // Add 1 to tail
                    containers[i].push_back(j);
                    break;
                case 2: // Remove head
                    if (!containers[i].empty()) containers[i].pop_front();
                    break;
                case 3: // Remove tail
                    if (!containers[i].empty()) containers[i].pop_back();
                    break;
                case 4: // Add 50 to head
                    for (int k = 0; k < 50; ++k) {
                        containers[i].push_front(k);
                    }
                    break;
                case 5: // Add 50 to tail
                    for (int k = 0; k < 50; ++k) {
                        containers[i].push_back(k);
                    }
                    break;
                case 6: {
                    int size = containers[i].size();
                    if (size > 0) {
                        int index = rand() % size;
                        volatile int x = containers[i][index];
                    }
                }
                    break;
                default:
                    std::cerr << "Wrong value for op!";
                    break;
            }

            // Trim the container if it grows too large
            if (containers[i].size() > 100) {
                for (int k = 0; k < 95; ++k) {
                    containers[i].pop_front();
                }
                containers[i].shrink_to_fit();

            } else if (containers[i].empty()) {
                containers[i].push_front(0);
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

void do_benchmark_openmp_dq(int operations) {
    vector<int> test_sizes = {0, 10, 50, 100, 1000};
    ofstream results_file("benchmark_results_openmp_dq.csv");
    results_file << "Test Size,Deque Type,Time (ms)\n";

    cout << "Benchmarking std::deque and ShiftToMiddleArray with OpenMP:\n";
    cout << "Operations: " << operations << "\n";
    cout << "Test sizes: ";
    for (int size : test_sizes) cout << size << " ";
    cout << "\n\n";

    for (int size : test_sizes) {
        double stdDequeTime = benchmark_parallel_containers<std::deque<int>>(size, operations);
        double stmArrayTime = benchmark_parallel_containers<ShiftToMiddleArray<int>>(size, operations);

        auto compute_speedup = [](double best, double stm) {
            return ((best - stm) / best) * 100;
        };

        double stm_speedup = compute_speedup(stdDequeTime, stmArrayTime);

        cout << "Test size: " << size << "\n";
        cout << "std::deque (Parallel): " << stdDequeTime << " ms\n";
        cout << "ShiftToMiddleArray (Parallel): " << stmArrayTime << " ms\n";
        cout << "ShiftToMiddleArray was " << abs(stm_speedup) << "% "
             << (stm_speedup < 0 ? "slower" : "faster") << " than std::deque.\n\n";

        results_file << size << ",std::deque," << stdDequeTime << "\n";
        results_file << size << ",ShiftToMiddleArray," << stmArrayTime << "\n";
    }
    results_file.close();
    cout << "Results saved to benchmark_results_openmp_dq.csv\n";
}
