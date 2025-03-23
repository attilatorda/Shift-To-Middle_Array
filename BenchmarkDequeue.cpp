#include <array>
#include <deque>
#include <queue>
#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include <fstream>
#include <omp.h>
#include "BenchmarkDequeue.h"
#include "ShiftToMiddleArray.h"
#include "ExpandingRingBuffer.h"

using namespace std;

template <typename DequeueType>
double benchmark_deque_growth(int size, int operations, const int iterations = 10) {
    std::mt19937 rng(42); // Fixed seed for reproducibility
    DequeueType dequeue(10);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        // Initial insertions
        for (int j = 0; j < size; ++j) {
            if (rng() % 2 == 0) dequeue.push_front(j);
            else dequeue.push_back(j);
        }

        // Mixed random operations
        for (int j = 0; j < operations; ++j) {
            switch (rng() % 4) {
                case 0: dequeue.push_front(j); break;
                case 1: dequeue.push_back(j); break;
                case 2: if (!dequeue.empty()) dequeue.pop_front(); break;
                case 3: if (!dequeue.empty()) dequeue.pop_back(); break;
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    double total_time = std::chrono::duration<double, std::milli>(end - start).count();
    return total_time;
}

void run_benchmarks_deque(int operations) {
    vector<int> test_sizes = {10, 100, 1000, 5000, 10000, 100000};
    int runs = 8; // Number of benchmark runs to average

    ofstream results_file("benchmark_results_deque.csv");
    results_file << "Size,Type,Time\n";

    cout << "Benchmarking different deque implementations:\n";
    cout << "Operations: " << operations << "\n";
    cout << "Container sizes: ";
    for (int size : test_sizes) cout << size << " ";
    cout << "\n\n";

    for (int size : test_sizes) {
        double stdDequeTotal = 0.0, erBufferTotal = 0.0, stmArrayTotal = 0.0;

        for (int i = 0; i < runs; ++i) {
            stdDequeTotal += benchmark_deque_growth<std::deque<int>>(size, operations);
            erBufferTotal += benchmark_deque_growth<ExpandingRingBuffer<int>>(size, operations);
            stmArrayTotal += benchmark_deque_growth<ShiftToMiddleArray<int>>(size, operations);

        }

        double stdDequeTime = stdDequeTotal / runs;
        double erBufferTime = erBufferTotal / runs;
        double stmArrayTime = stmArrayTotal / runs;

        auto compute_speedup = [](double best, double stm) {
            return ((best - stm) / best) * 100;
        };

        double best_time = min(stdDequeTime, erBufferTime);
        double stm_speedup = compute_speedup(best_time, stmArrayTime);

        cout << "Container size: " << size << "\n";
        cout << "std::deque (avg over " << runs << " runs): " << stdDequeTime << " ms\n";
        cout << "ExpandingRingBuffer (avg over " << runs << " runs): " << erBufferTime << " ms\n";
        cout << "ShiftToMiddleArray (avg over " << runs << " runs): " << stmArrayTime << " ms\n";
        cout << "ShiftToMiddleArray was " << abs(stm_speedup) << "% "
             << (stm_speedup < 0 ? "slower" : "faster") << " than the best alternative.\n";

        results_file << size << ",std::deque," << stdDequeTime << "\n";
        results_file << size << ",ExpandingRingBuffer," << erBufferTime << "\n";
        results_file << size << ",ShiftToMiddleArray," << stmArrayTime << "\n";
    }
    results_file.close();
    cout << "Results saved to benchmark_results_deque.csv\n";
}
