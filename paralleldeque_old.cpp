template <typename DequeueType>
double benchmark_deque_parallel_operations(int start_size, int operations, const int num_deques = 100) {

    std::vector<DequeueType> deques(num_deques);

    start_size /= num_deques;

    //operations = (operations * 10) / num_deques;

    for (int i = 0; i < num_deques; ++i) {
        for (int j = 0; j < start_size; ++j) {
            deques[i].push_front(j);
        }
    }

    int spike_no = start_size > 100 ? start_size / 20 : 5;

    auto start = std::chrono::high_resolution_clock::now();

    // Parallel processing
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < num_deques; ++i) {
        std::mt19937 local_rng(42 + i); // Unique seed per thread to avoid contention
        std::discrete_distribution<int> local_op_dist({15, 15, 20, 20, 5, 5, 20});

        for (int j = 0; j < operations; ++j) {
            int op = local_rng() % 7;  // Use local RNG

            if (op == 0) deques[i].push_front(j);  // Add 1 to head
            else if (op == 1) deques[i].push_back(j); // Add 1 to tail
            else if (op == 2 && !deques[i].empty()) deques[i].pop_front(); // Remove head
            else if (op == 3 && !deques[i].empty()) deques[i].pop_back(); // Remove tail
            else if (op == 4) { //Spike add front
                for (int k = spike_no; k > 0; --k) deques[i].push_front(k);
            }
            else if (op == 5) { //Spike add back
                for (int k = spike_no; k > 0; --k) deques[i].push_back(k);
            }
            else if (op == 6) { // Read random
                int size = deques[i].size();
                if (size > 0) {
                    deques.front();
                }
            }

            // Clean up deque to simulate realistic operation size
            if (deques[i].size() > 100) {
                for (int k = 0; k < 95; ++k) deques[i].pop_front();
            } else if (deques[i].empty()) {
                deques[i].push_front(0);
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    double total_time = std::chrono::duration<double, std::milli>(end - start).count();

    return total_time;
}
