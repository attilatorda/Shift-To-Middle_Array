# Shift-To-Middle Array

The **Shift-To-Middle Array** is a dynamic array designed to optimize **insertions and deletions at both ends**, offering a high-performance alternative to implementing dynamic arrays, queues, dequeues. It achieves this while maintaining **contiguous memory storage**, improving **cache locality** and enabling **efficient parallel processing**.

![Shift-To-Middle Array](stm.png)

## Features

**-Amortized O(1) insertions and O(1) deletions at both ends**  
**-Fast random access (O(1))**  
**-Better cache locality than linked lists**  
**-Supports SIMD & parallel optimizations**  
**-Minimizes memory overhead and avoids fragmentation unlike std::deque** <br>
**-Dynamic biasing for push-heavy workloads (#define BIAS_MULT)** <br>
**-Manual shrink_to_fit() to reclaim unused memory** <br>
**-Optional automatic shrinking**

## How It Works

To overcome the costly front-insertions of dynamic arrays and the non-contiguous memory of linked lists, the Shift-To-Middle Array proactively re-balances its data. During a resize, it shifts the contents to the middle, ensuring ample room for growth on both ends and minimizing future copy operations, without sacrificing the performance benefits of contiguous memory.

## Time Complexity Comparison

The following table compares the time complexity of Shift-To-Middle Array operations with other common data structures:

| Operation                  | Dynamic Array           | Linked List | Shift-To-Middle Array |
|---------------------------|--------------------------|-------------|-----------------------|
| Access (by index)          | O(1)                     | O(n)        | O(1)                 |
| Insertion at head          | O(n)                     | O(1)        | O(1) amortized       |
| Insertion at tail          | O(1) amortized           | O(1)        | O(1) amortized       |
| Insertion in middle        | O(n)                     | O(n)        | O(n)                 |
| Deletion at head           | O(n)                     | O(1)        | O(1)                 |
| Deletion at tail           | O(1)                     | O(1)        | O(1)                 |
| Deletion in middle         | O(n)                     | O(n)        | O(n)                 |
| Cache Locality             | Excellent                | Poor        | Excellent            |

Note: Deletion at head and tail are O(1) if shrinking is disabled, and O(1) amortized if shrinking is enabled (since resizing only occurs occasionally).

## Theoretical Properties of STM Array

| Operation          | Best Case         | Average Case                | Worst Case          | Notes                          |
|--------------------|-------------------|-----------------------------|---------------------|--------------------------------|
| **Random Access**  | Θ(1)             | Θ(1)                       | Θ(1)               | Direct block indexing          |
| **Insert at Front/End**  | Θ(1)             | Θ(1) amortized              | Θ(n)               | Occurs during full expansion   |
| **Delete at Front/End**  | Θ(1)             | Θ(1)                       | Θ(1)               | No shrinkage implemented       |
| **Middle Insert**  | Θ(n)             | Θ(n/2) ≈ Θ(n)                    | Θ(n)       | On average shifts half the elements due to central bias     |
| **Middle Delete**  | Θ(n)             | Θ(n/2) ≈ Θ(n)                    | Θ(n)       | On average shifts half the elements due to central bias     |

#### Key to Notations:
- **Θ(f(n))**: Tight bound (both upper and lower)
- **O(f(n))**: Upper bound
- **o(f(n))**: Strictly better than O(f(n))

## Performance Benchmarks
Benchmarks comparing **Shift-To-Middle Array vs. `std::deque` vs. ExpandingRingBuffer vs. `std::queue`** demonstrate that performance improvements depend on **CPU and GPU capabilities**, such as **multi-core parallelism, SIMD optimizations, and cache efficiency**.

The benchmarks were compiled using **GCC with the `-O3` optimization flag**, ensuring high-performance execution. Results vary based on **hardware specifications** and **workload characteristics**. <br>

-AMD Ryzen 5 2600X Six-Core Processor <br>
-NVIDIA GeForce GTX 1660 SUPER <br>
-16GB RAM <br>

For full benchmark details, check out the [publication](ShiftToMiddleArray.pdf). The provided **Python scripts** can be used to visualize performance metrics from CSV benchmark results.

**Note**: Performances are being reevaluated as I'm making a new version of the publication!

## Installation & Compilation
The Shift-To-Middle Array is a single-header, templated C++ class. To use it, simply include ShiftToMiddleArray.h in your project. Requirements: C++ 20 or later and a standards-compliant compiler. I recommend to compile with these flags: <br>

```sh
g++ -std=c++20 -Ofast -Wall -Wextra -Werror -pedantic main.cpp BenchmarkQueue.cpp BenchmarkDequeue.cpp BenchmarkList.cpp -o queue_benchmarks
```

To run the **Java benchmarks**, ensure you have the **Trove library** installed. Compile and execute using:
```sh
javac -cp trove-3.0.3.jar; ShiftToMiddleArrayBenchmarkTrove.java
java -cp trove-3.0.3.jar; ShiftToMiddleArrayBenchmarkTrove
```

## 🔬 Possible Applications

- **Embedded Systems**
- **Game Engines & Real-Time Applications**
- **Backend Frameworks**
- **Scientific Computing**

## History

The Shift-To-Middle Array was developed by Attila Torda as a personal project during free time, aiming to create a more efficient implementation strategy for lists and deques. This project explored whether a contiguous-memory approach with dynamic mid-shifting could offer better balance for insertions, deletions, and random access. I wrote a white paper and wanted to publish it, but got rejected.

- **Designed and implemented without academic/financial support, relying on open-source tools and iterative testing.**
- **Initial results showed promise, with one ICCS 2025 reviewer noting the method’s "reliability" compared to ArrayLists/linked lists (Score: 4/5 on results presentation).**
- **Areas for improvement identified: deeper benchmarking and formal literature review.**

After the rejection for publication I implemented the **bias** feature. I plan to make more tests, benchmarks and improvements, then try to publish it again.

## FAQ

**Did you reinvent the array-based deque?**

Yes and no! The Shift-to-Middle (STM) Array is an implementation strategy that can be used for various data structures, including lists, queues, and deques. While both STM arrays and traditional array deques use array-backed storage, they optimize for fundamentally different operations and access patterns.

A traditional array deque is optimized for operations at its ends (head and tail), providing O(1) performance for adding or removing elements there. However, inserting or deleting elements in the middle requires shifting a large number of elements, resulting in O(n) time complexity.

In contrast, the STM Array also provides O(1) performance at the ends but achieves amortized O(1) performance even for insertions and deletions in the middle through its unique shift-to-middle approach.

One of the key innovations is the bias system, which allows the STM Array structure to dynamically optimize its internal memory layout based on observed usage patterns – something traditional array deques cannot do. Where an array deque is fixed in its end-optimized behavior, the STM Array automatically adapts to whether the application performs mostly front, middle, or back operations.

This makes the STM Array strictly more versatile. It can efficiently handle all the queue and deque use cases typical of an array deque, while also supporting efficient list-like operations in the middle that would be prohibitively expensive with a traditional array deque.

**Did you use outside help or AI for this project?**

I did! The project benefited from both AI assistance and community feedback.

AI Help: Even the name "Shift-To-Middle Array" was AI-generated! However, I encountered challenges—when my source files grew large, AI chatbots often made mistakes, so I had to carefully review each modification.

Community Feedback: This project also improved thanks to input from the Hacker News and Reddit (r/algorithms) communities. Their technical insights helped refine the implementation. Special shoutout to these discussions:

[Hacker News thread on data structure efficiency](https://news.ycombinator.com/item?id=43456669) <br>
[Reddit r/algorithms feedback discussion](https://www.reddit.com/r/algorithms/comments/1jix7zi/comment/mjtou49/?context=3)

## Variations

**Fixed-Size Implementation** <br>
A simple, memory-efficient version where the array has a predefined maximum capacity.

**Unrolled Shift-to-Middle Array** <br>
A hybrid between an unrolled linked list and a shift-to-middle (STM) array, balancing cache efficiency and dynamic operations.

**Shift-to-Middle Ring Buffer** <br>
A ring buffer whose head and tail are moved to the middle. Resizing could occur when the array is full, or when a certain condition is met, ie. an insert operation is requested while the head is already behind the tail and the array is 80% full.

**Hybrid Array Combinations** <br>
It's possible to combine different array implementations (ring buffer, STM array and unrolled linked list) to create a hybrid aray, which changes implementations at different array sizes.

## License

This project is distributed under the terms of the AGPLv3 License. The full license text can be found in the  [LICENSE](LICENSE) file. Please reach out to me on [LinkedIn](https://www.linkedin.com/in/attila-torda-787503a5/) to inquire about purchasing a different license.

## AI Training Ban

You are **not permitted** to use this repository or its contents for training any machine learning or AI models without prior written consent. This includes uploading to datasets, AI corpora, or any derivative datasets used for training.
