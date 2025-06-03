# Shift-To-Middle Array

The **Shift-To-Middle Array** is a dynamic array designed to optimize **insertions and deletions at both ends**, offering a high-performance alternative to implementing dynamic arrays, queues, dequeues. It achieves this while maintaining **contiguous memory storage**, improving **cache locality** and enabling **efficient parallel processing**.

![Shift-To-Middle Array](stm.png)

## 🌟 Features

✅ **Amortized O(1) insertions & deletions at both ends**  
✅ **Fast random access (O(1))**  
✅ **Better cache locality than linked lists**  
✅ **Supports SIMD & parallel optimizations**  
✅ **Minimizes memory overhead and avoids fragmentation unlike std::deque** <br>
✅ **Dynamic biasing for push-heavy workloads (#define BIAS_MULT)** <br>
✅ **Manual shrink_to_fit() to reclaim unused memory** <br>
✅ **Optional bounds checking** <br>
✅ **Optional automatic shrinking**

## 📌 How It Works

Traditional dynamic arrays often suffer from costly shifts when inserting at the front, and structures like std::deque rely on fragmented memory blocks to mitigate this. The Shift-To-Middle Array takes a different approach: it dynamically re-centers data during resizing, ensuring balanced space on both ends and minimizing copying — all while maintaining a contiguous memory layout.

## 🚀 Time Complexity Comparison

The following table compares the time complexity of Shift-To-Middle Array operations with other common data structures:

| Operation                  | Dynamic Array           | Linked List | Shift-To-Middle Array |
|---------------------------|--------------------------|-------------|-----------------------|
| Access (by index)          | O(1)                     | O(n)        | O(1)                 |
| Insertion at head          | O(n)                     | O(1)        | O(1) amortized       |
| Insertion at tail          | O(1) amortized           | O(1)        | O(1) amortized       |
| Insertion in middle        | O(n)                     | O(n)        | O(n)                 |
| Deletion at head           | O(n)                     | O(1)        | O(1) amortized       |
| Deletion at tail           | O(1)                     | O(1)        | O(1) amortized       |
| Deletion in middle         | O(n)                     | O(n)        | O(n)                 |
| Cache Locality             | Excellent                | Poor        | Excellent            |

## 📙Theoretical Properties of STM Array

| Operation          | Best Case         | Average Case                | Worst Case          | Notes                          |
|--------------------|-------------------|-----------------------------|---------------------|--------------------------------|
| **Random Access**  | Θ(1)             | Θ(1)                       | Θ(1)               | Direct block indexing          |
| **Insert at End**  | Θ(1)             | Θ(1) amortized              | Θ(n)               | Occurs during full expansion   |
| **Delete at End**  | Θ(1)             | Θ(1)                       | Θ(1)               | No shrinkage implemented       |
| **Middle Insert**  | Θ(1)             | o(n) amortized              | Θ(n)               | With bias optimization         |
| **Middle Delete**  | Θ(1)             | o(n) amortized              | Θ(n)               | With bias optimization         |
| **Bias Update**    | Θ(1)             | O(1) expected               | O(log n)           | Count-min sketch probabilistic |
| **Rebalance**      | -                | O(√n) amortized             | O(n)               | Full structure reorganization  |

#### Key to Notations:
- **Θ(f(n))**: Tight bound (both upper and lower)
- **O(f(n))**: Upper bound
- **o(f(n))**: Strictly better than O(f(n))
- **Amortized**: Average over sequence of operations
- **Expected**: Average under reasonable distribution

#### Special Cases:
1. **Clustered Middle Ops**:  
   - Achieves O(1) average when ≥k consecutive middle operations occur (geometric distribution)

2. **Stable Workloads**:  
   - Bias system reduces average middle ops to o(√n) after convergence

3. **Adversarial Patterns**:  
   - Worst-case Θ(n) only occurs with:  
     - Alternating end/middle ops  
     - Malicious bias thrashing  

**Note**: I'm still reviewing this section!

## 🏆 Performance Benchmarks
Benchmarks comparing **Shift-To-Middle Array vs. `std::deque` vs. ExpandingRingBuffer vs. `std::queue`** demonstrate that performance improvements depend on **CPU and GPU capabilities**, such as **multi-core parallelism, SIMD optimizations, and cache efficiency**.

The benchmarks were compiled using **GCC with the `-O3` optimization flag**, ensuring high-performance execution. Results vary based on **hardware specifications** and **workload characteristics**. <br>

-AMD Ryzen 5 2600X Six-Core Processor <br>
-NVIDIA GeForce GTX 1660 SUPER <br>
-16GB RAM <br>

## 📂 Installation & Compilation
The Shift-To-Middle Array is a single-header, templated C++ class. To use it, simply include ShiftToMiddleArray.h in your project. Requirements: C++ 20 or later and a standards-compliant compiler. I recommend to compile with these flags: <br>

```sh
g++ -std=c++20 -O3 -Wall -Wextra -pedantic -fopenmp 
```

To run the **Java benchmarks**, ensure you have the **Trove library** installed. Compile and execute using:
```sh
javac -cp trove-3.0.3.jar; ShiftToMiddleArrayBenchmarkTrove.java
java -cp trove-3.0.3.jar; ShiftToMiddleArrayBenchmarkTrove
```

## 🔬 Possible Applications

- **Embedded systems**
- **Game engines & real-time applications**
- **Backend Frameworks**
- **Scientific Computing**

## 📊 Benchmarks & Results

For full benchmark details, check out the [publication](ShiftToMiddleArray.pdf). The provided **Python scripts** can be used to visualize performance metrics from CSV benchmark results.

## 🏛 History

The Shift-To-Middle Array was independently developed by Attila Torda as a personal project during free time, aiming to create a more efficient implementation strategy for lists and deques. This project explored whether a contiguous-memory approach with dynamic mid-shifting could offer better balance for insertions, deletions, and random access.

- **Designed and implemented without academic/financial support, relying on open-source tools and iterative testing.**
- **Initial results showed promise, with one ICCS 2025 reviewer noting the method’s "reliability" compared to ArrayLists/linked lists (Score: 4/5 on results presentation).**
- **Areas for improvement identified: deeper benchmarking and formal literature review.**

After the publication I implemented the **bias** feature. I plan to make more tests, benchmarks and improvements, then planning to port the data structure to other languages and ecosystems.

## ❓ FAQ

**Is this just a ring buffer?** <br>
No — a ring buffer is a fixed-size circular structure that wraps around when full. The Shift-To-Middle Array is dynamically resizable and allows both ends to grow without wrapping. Unlike ring buffers, it can handle arbitrary growth while maintaining amortized O(1) operations and fast random access.

**Did you reinvent the array-based deque?**

No. The Shift-to-Middle (STM) Array is an implementation strategy that can be used for various data structures, including lists, queues, and deques. While both STM arrays and traditional array deques use array-backed storage, they optimize for fundamentally different operations and access patterns.

A traditional array deque is optimized for operations at its ends (head and tail), providing O(1) performance for adding or removing elements there. However, inserting or deleting elements in the middle requires shifting a large number of elements, resulting in O(n) time complexity.

In contrast, the STM Array also provides O(1) performance at the ends but achieves amortized O(1) performance even for insertions and deletions in the middle through its unique shift-to-middle approach.

The key innovation is the bias system, which allows the STM Array structure to dynamically optimize its internal memory layout based on observed usage patterns – something traditional array deques cannot do. Where an array deque is fixed in its end-optimized behavior, the STM Array automatically adapts to whether the application performs mostly front, middle, or back operations.

This makes the STM Array strictly more versatile. It can efficiently handle all the queue and deque use cases typical of an array deque, while also supporting efficient list-like operations in the middle that would be prohibitively expensive with a traditional array deque.

**Why not resize with an asymmetric buffer if most operations are push_front or push_back?** <br>
You can! The Shift-To-Middle Array supports dynamic biasing via a bias parameter. When enabled (with #define BIAS_MULT), the buffer adjusts headroom during resizing based on recent usage patterns — giving more space to the side you're actively pushing to. It's automatic and tunable.

**Is this better than std::deque?** <br>
For many cases, yes: <br>
✅Contiguous memory improves cache performance. <br>
✅No fragmentation simplifies allocator usage. <br>
✅Easier SIMD and memory-aligned optimizations.

**Will this ever be in the STL?** <br>
Unlikely — the STL already has std::deque, std::vector, std::list, etc., and avoids redundancy. But this structure could live in performance-focused libraries or game engines where custom memory layouts matter.

**Did you use AI?** <br>
Yes, I did! Even the name "Shift-To-Middle Array" was generated by AI! However, all the AI Chatbots I used kept making mistakes, especially when my source files got big, so I had to carefully review each modification.

## 🔄 Variations

**Fixed-Size Implementation** <br>
A simple, memory-efficient version where the array has a predefined maximum capacity.

**Unrolled Shift-to-Middle Array** <br>
A hybrid between an unrolled linked list and a shift-to-middle (STM) array, balancing cache efficiency and dynamic operations.

**Probabilistic Version** <br>
For scenarios favoring speed over strict correctness, this variant introduces probabilistic techniques (e.g., Bloom filters or count-min sketches) to approximate queries. It trades exactness for O(1) lookups and reduced memory usage, ideal for applications like caching, network routing, or streaming algorithms where occasional false positives/negatives are acceptable.

## 📜 License

This project is distributed under the terms of the AGPLv3 License. The full license text can be found in the  [LICENSE](LICENSE) file. For companies requiring a commercial license with different terms than the AGPLv3, I offer a perpetual license for a single payment of 50 euros per company, allowing internal use without resale rights. Please reach out to me on [LinkedIn](https://www.linkedin.com/in/attila-torda-787503a5/) to inquire about purchasing.

For major contributions, I'm happy to offer a free license for your company (non-resale use only). This offer is not legally binding, and I reserve the right to determine what qualifies as a major contribution.

## 🚫🤖 AI Training Ban

You are **not permitted** to use this repository or its contents for training any machine learning or AI models without prior written consent. This includes uploading to datasets, AI corpora, or any derivative datasets used for training.

## 🙏 Acknowledgments

This project benefited greatly from community feedback! Special thanks to:

- **Hacker News** and **Reddit's r/algorithms** communities for their technical insights and suggestions that helped refine the core ideas.

**Relevant discussions:**
- [Hacker News thread on data structure efficiency](https://news.ycombinator.com/item?id=43456669)
- [Reddit r/algorithms feedback discussion](https://www.reddit.com/r/algorithms/comments/1jix7zi/comment/mjtou49/?context=3)

## 🤝 Contributing

Contributions are welcome! Feel free to open an issue or pull request.
