#include <array>
#include <deque>
#include <queue>
#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include <fstream>
#include <omp.h>
#include <cmath>
#include "BenchmarkDequeue.h"
#include "ShiftToMiddleArray.h"
#include "ExpandingRingBuffer.h"

static double mean_of(const std::vector<double>& v) {
    double s = 0.0;
    for (double x : v) s += x;
    return v.empty() ? 0.0 : s / static_cast<double>(v.size());
}

static double stddev_of(const std::vector<double>& v, double mean) {
    if (v.size() < 2) return 0.0;
    double ss = 0.0;
    for (double x : v) {
        const double d = x - mean;
        ss += d * d;
    }
    return std::sqrt(ss / static_cast<double>(v.size() - 1));
}

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
    std::vector<int> test_sizes = {10, 100, 1000, 5000, 10000, 100000};
    int runs = 8; // Number of benchmark runs to average

    std::ofstream results_file("benchmark_results_deque.csv");
    results_file << "Size,Type,TimeMeanMs,TimeStdMs\n";

    std::cout << "Benchmarking different deque implementations:\n";
    std::cout << "Operations: " << operations << "\n";
    std::cout << "Container sizes: ";
    for (int size : test_sizes) std::cout << size << " ";
    std::cout << "\n\n";

    for (int size : test_sizes) {
        std::vector<double> stdDequeTimes, erBufferTimes, stmArrayTimes;
        stdDequeTimes.reserve(runs);
        erBufferTimes.reserve(runs);
        stmArrayTimes.reserve(runs);

        for (int i = 0; i < runs; ++i) {
            stdDequeTimes.push_back(benchmark_deque_growth<std::deque<int>>(size, operations));
            erBufferTimes.push_back(benchmark_deque_growth<ExpandingRingBuffer<int>>(size, operations));
            stmArrayTimes.push_back(benchmark_deque_growth<ShiftToMiddleArray<int>>(size, operations));
        }

        double stdDequeTime = mean_of(stdDequeTimes);
        double erBufferTime = mean_of(erBufferTimes);
        double stmArrayTime = mean_of(stmArrayTimes);
        double stdDequeStd = stddev_of(stdDequeTimes, stdDequeTime);
        double erBufferStd = stddev_of(erBufferTimes, erBufferTime);
        double stmArrayStd = stddev_of(stmArrayTimes, stmArrayTime);

        auto compute_speedup = [](double best, double stm) {
            return ((best - stm) / best) * 100;
        };

        double best_time = std::min(stdDequeTime, erBufferTime);
        double stm_speedup = compute_speedup(best_time, stmArrayTime);

        std::cout << "Container size: " << size << "\n";
        std::cout << "std::deque (avg over " << runs << " runs): " << stdDequeTime << " ms\n";
        std::cout << "ExpandingRingBuffer (avg over " << runs << " runs): " << erBufferTime << " ms\n";
        std::cout << "ShiftToMiddleArray (avg over " << runs << " runs): " << stmArrayTime << " ms\n";
        std::cout << "ShiftToMiddleArray was " << std::abs(stm_speedup) << "% "
                  << (stm_speedup < 0 ? "slower" : "faster") << " than the best alternative.\n";

        results_file << size << ",std::deque," << stdDequeTime << "," << stdDequeStd << "\n";
        results_file << size << ",ExpandingRingBuffer," << erBufferTime << "," << erBufferStd << "\n";
        results_file << size << ",ShiftToMiddleArray," << stmArrayTime << "," << stmArrayStd << "\n";
    }
    results_file.close();
    std::cout << "Results saved to benchmark_results_deque.csv\n";
}
