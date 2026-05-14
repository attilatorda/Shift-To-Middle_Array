#include "BenchmarkQueue.h"
#include <cmath>

using namespace std;

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

template <typename QueueType, typename InitFunc, typename OperationFunc>
double benchmark_queue(int size, int operations, InitFunc init, OperationFunc operation) {
    double total_time = 0;
    int runs = 20;

    for (int i = 0; i < runs; ++i) {
        QueueType queue;  // Create queue instance
        init(queue, size); // Initialize before timing

        auto start = chrono::high_resolution_clock::now();
        operation(queue, operations); // Perform operations on the same queue
        auto end = chrono::high_resolution_clock::now();

        total_time += chrono::duration<double, milli>(end - start).count();
    }
    return total_time / double (runs);
}

template <typename QueueType>
void init_queue(QueueType& queue, int test_size) {
    for (int i = 0; i < test_size; ++i) {
        queue.push(i);
    }
}

template <typename QueueType>
std::array<double, 3> benchmark_all(int test_size, int operations) {
    std::mt19937 rng(42); // Fixed seed for reproducibility
    std::array<double, 3> queue_time;

    queue_time[0] = benchmark_queue<QueueType>(test_size, operations, init_queue<QueueType>,
        [&](QueueType& queue, int operations) {
            for (int i = 0; i < operations; ++i) {
                if (rng() % 10 < 8) queue.push(i);//80% Push, 20% Pop
                else if(!queue.empty()) queue.pop();
            }
        });

    queue_time[1] = benchmark_queue<QueueType>(test_size, operations, init_queue<QueueType>,
        [&](QueueType& queue, int operations) {
            for (int i = 0; i < operations; ++i) {
                if (rng() % 10 < 5) queue.push(i);//50% Push, 50% Pop
                else if(!queue.empty()) queue.pop();
            }
        });

    queue_time[2] = benchmark_queue<QueueType>(test_size, operations, init_queue<QueueType>,
        [&](QueueType& queue, int operations) {
            for (int i = 0; i < operations; ++i) {
                if (rng() % 10 < 2) queue.push(i);//20% Push, 80% Pop
                else if(!queue.empty()) queue.pop();
            }
        });

    return queue_time;
}

void run_benchmarks_queue(int operations) {
    vector<int> test_sizes = {0, 100, 1000, 10000, 50000, 100000, 500000, 1000000};
    int runs = 8; // Number of benchmark runs to average

    ofstream results_file("benchmark_results_queue.csv");
    results_file << "Size,Type,PushHeavyMeanMs,PushHeavyStdMs,MixedMeanMs,MixedStdMs,PopHeavyMeanMs,PopHeavyStdMs\n";

    cout << "Benchmarking different queue implementations: \n";
    cout << "Operations: " << operations << "\n";
    cout << "Container sizes: ";
    for (int size : test_sizes) cout << size << " ";
    cout << "\n\n";

    for (int size : test_sizes) {
        std::array<std::vector<double>, 3> stdQueueRuns;
        std::array<std::vector<double>, 3> erBufferRuns;
        std::array<std::vector<double>, 3> stmArrayRuns;
        for (int j = 0; j < 3; ++j) {
            stdQueueRuns[j].reserve(runs);
            erBufferRuns[j].reserve(runs);
            stmArrayRuns[j].reserve(runs);
        }

        for (int i = 0; i < runs; ++i) {
            auto results1 = benchmark_all<std::queue<int>>(size, operations);
            auto results2 = benchmark_all<ExpandingRingBuffer<int>>(size, operations);
            auto results3 = benchmark_all<ShiftToMiddleArray<int>>(size, operations);

            for (int j = 0; j < 3; ++j) {
                stdQueueRuns[j].push_back(results1[j]);
                erBufferRuns[j].push_back(results2[j]);
                stmArrayRuns[j].push_back(results3[j]);
            }
        }

        std::array<double, 3> stdQueue, erBuffer, stmArray;
        std::array<double, 3> stdQueueStd, erBufferStd, stmArrayStd;
        for (int j = 0; j < 3; ++j) {
            stdQueue[j] = mean_of(stdQueueRuns[j]);
            erBuffer[j] = mean_of(erBufferRuns[j]);
            stmArray[j] = mean_of(stmArrayRuns[j]);
            stdQueueStd[j] = stddev_of(stdQueueRuns[j], stdQueue[j]);
            erBufferStd[j] = stddev_of(erBufferRuns[j], erBuffer[j]);
            stmArrayStd[j] = stddev_of(stmArrayRuns[j], stmArray[j]);
        }

        cout << "Test size: " << size << "\n";
        cout << "std::queue - Push-heavy: " << stdQueue[0] << " ms, Mixed: " << stdQueue[1] << " ms, Pop-heavy: " << stdQueue[2] << " ms\n";
        cout << "ExpandingRingBuffer - Push-heavy: " << erBuffer[0] << " ms, Mixed: " << erBuffer[1] << " ms, Pop-heavy: " << erBuffer[2] << " ms\n";
        cout << "ShiftToMiddleArray - Push-heavy: " << stmArray[0] << " ms, Mixed: " << stmArray[1] << " ms, Pop-heavy: " << stmArray[2] << " ms\n";

        for (int j = 0; j < 3; ++j) {
            double best_time = min(stdQueue[j], erBuffer[j]);
            double stm_speedup = ((best_time - stmArray[j]) / best_time) * 100;
            cout << "ShiftToMiddleArray was " << abs(stm_speedup) << "% "
                 << (stm_speedup < 0 ? "slower" : "faster") << " than the best alternative.\n";
        }
        cout << "\n";

        results_file << size << ",std::queue," << stdQueue[0] << "," << stdQueueStd[0] << ","
                     << stdQueue[1] << "," << stdQueueStd[1] << ","
                     << stdQueue[2] << "," << stdQueueStd[2] << "\n";
        results_file << size << ",ExpandingRingBuffer," << erBuffer[0] << "," << erBufferStd[0] << ","
                     << erBuffer[1] << "," << erBufferStd[1] << ","
                     << erBuffer[2] << "," << erBufferStd[2] << "\n";
        results_file << size << ",ShiftToMiddleArray," << stmArray[0] << "," << stmArrayStd[0] << ","
                     << stmArray[1] << "," << stmArrayStd[1] << ","
                     << stmArray[2] << "," << stmArrayStd[2] << "\n";
    }

    results_file.close();
    cout << "Results saved to benchmark_results_queue.csv\n";
}
