#include <iostream>
#include <deque>
#include <vector>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include "ShiftToMiddleArray.h"
#include "ExpandingRingBuffer.h"
#include "BenchmarkDequeue.h"
#include "BenchmarkQueue.h"
#include "BenchmarkList.h"

void checkValidity() {
    ShiftToMiddleArray<int> stmArray;
    std::queue<int> m_queue;
    ExpandingRingBuffer<int> erf;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 4);

    for (int i = 0; i < 10000; ++i) {
        int value = rng();
        if (dist(rng) < 4) { // Push operation
            stmArray.insert_tail(value);
            m_queue.push(value);
            erf.push(value);
        } else if (!m_queue.empty()) { // Pop operation
            int v1 = stmArray.get_head(); stmArray.remove_head();
            int v2 = m_queue.front(); m_queue.pop();
            int v3 = erf.front(); erf.pop();

            if (v1 != v2 || v2 != v3) {
                std::cerr << "Mismatch detected! " << v1 << " != " << v2 << " != " << v3 << std::endl;
                exit(0);
                return;
            }
        }
    }

    // Check validity using direct access
    if (stmArray.size() != m_queue.size() || m_queue.size() != erf.size()) {
        std::cerr << "Size mismatch detected!" << std::endl;
        exit(0);
        return;
    }

    for (size_t i = 0; i < stmArray.size(); ++i) {
        if (stmArray[i] != erf[i]) {
            std::cerr << "Mismatch detected at index " << i << "! "
                      << stmArray[i] << " != " << erf[i] << std::endl;
            exit(0);
            return;
        }
    }

    std::cout << "All structures behaved identically." << std::endl;
}


int main() {

    std::cout << "C++ version: GCC " << __cplusplus << std::endl;
    unsigned int cores = std::thread::hardware_concurrency();
    if (cores == 0) cores = 1; // fallback
    std::cout << "Number of threads: " << cores << std::endl;

    checkValidity();

    run_benchmarks_queue(40000);
    run_benchmarks_deque(40000);
    run_benchmarks_list(100000);

    return 0;
}
