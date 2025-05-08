#include <vector>
#include <list>
#include <random>
#include <chrono>
#include <iostream>
#include <fstream>
#include "BenchmarkList.h"
#include "ShiftToMiddleArray.h"

double benchmark_random_operations_list(int size, int operations, const int iterations) {
    std::mt19937 rng(42); // Fixed seed for reproducibility
    std::discrete_distribution<int> op_dist({30, 30, 30, 10}); // 10% chance for spike

    std::list<int> container;
    bool spikeMode = false;
    [[maybe_unused]] volatile int stored_value = 0; // Prevent compiler optimizations

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
            auto it = container.begin();
            std::advance(it, index);

            int op = op_dist(rng);
            switch (op) {
                case 0: // Insert at random position
                    container.insert(it, j);
                    break;
                case 1: // Remove if not empty
                    container.erase(it);
                    break;
                case 2: // Read element
                    stored_value = *it;
                    break;
                case 3: { // Spike event: randomly remove/add 10% of elements
                    int spike_size = container.size() / 10;
                    for (int k = 0; k < spike_size; ++k) {
                        int spike_index = rng() % container.size();
                        auto spike_it = container.begin();
                        std::advance(spike_it, spike_index);

                        if (spikeMode && !container.empty()) {
                            container.erase(spike_it);
                        } else {
                            container.insert(spike_it, k);
                        }
                    }
                    spikeMode = !spikeMode; // Alternate spike behavior
                    break;
                }
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

void run_benchmarks_list(int operations) {
    vector<int> test_sizes = {10, 100, 1000, 5000, 10000, 100000, 500000};
    int runs = 8; // Number of benchmark runs to average

    ofstream results_file("benchmark_results_list.csv");
    results_file << "Size,Type,Time\n";

    for (int size : test_sizes) {
        cout << "Container size: " << size << "\n";

        double vector_total_time = 0.0;
        double stm_total_time = 0.0;

        for (int i = 0; i < runs; ++i) {
            vector_total_time += benchmark_random_operations<std::vector<int>>(size, operations);
            stm_total_time += benchmark_random_operations<ShiftToMiddleArray<int>>(size, operations);
        }

        double vector_avg_time = vector_total_time / runs;
        double stm_avg_time = stm_total_time / runs;

        cout << "Benchmarking std::vector...\n";
        cout << "std::vector (avg over " << runs << " runs): " << vector_avg_time << " ms\n";

        cout << "Benchmarking ShiftToMiddleArray...\n";
        cout << "ShiftToMiddleArray (avg over " << runs << " runs): " << stm_avg_time << " ms\n\n";

        double speedup = ((vector_avg_time - stm_avg_time) / vector_avg_time) * 100;
        cout << "ShiftToMiddleArray was " << abs(speedup) << "% "
             << (speedup < 0 ? "slower" : "faster") << " than std::vector.\n\n";

        //std::list was too slow, so it was ignored

        results_file << size << ",std::vector," << vector_avg_time << "\n";
        results_file << size << ",ShiftToMiddleArray," << stm_avg_time << "\n";
    }

    results_file.close();
    cout << "Results saved to benchmark_results_list.csv\n";
}
