#pragma once
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <omp.h>
#include <immintrin.h> // AVX/SSE intrinsics
#include <xmmintrin.h> // SSE intrinsics


template <typename T>
class ShiftToMiddleArray {
private:
    T* data;
    int head, tail, capacity;

    void resize() {

        int size = tail - head;

        /*if (size < capacity - 2) {
            shift(size);
            return;
        }*/

        int new_capacity = capacity * 2;
        T* new_data = (T*)std::malloc(new_capacity * sizeof(T));
        if (!new_data) {
            std::cerr << "Memory allocation failed!" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        int new_head = (new_capacity - (tail - head)) / 2;

        // Uncomment for OpenMP parallelization
        //#pragma omp parallel for
        //for (int i = head; i < tail; ++i) {
        //    new_data[new_head + (i - head)] = data[i];
        //}

        std::copy(data + head, data + tail, new_data + new_head);

        //#pragma omp parallel for
        //for (int i = 0; i < size; i += 8) {
        //    __m256i chunk = _mm256_loadu_si256((__m256i*)&src[i]);
        //    _mm256_storeu_si256((__m256i*)&dest[i], chunk);
        //}


        std::free(data);
        data = new_data;
        tail = new_head + (tail - head);
        head = new_head;
        capacity = new_capacity;
    }

    void shift(int size) {
        int new_head = (capacity - size) / 2; // Shift data to the middle
        int new_tail = new_head + size;

        std::memmove(data + new_head, data + head, size * sizeof(T));

        head = new_head;
        tail = new_tail;
    }

public:
    ShiftToMiddleArray(int initial_capacity = 8)
        : capacity(initial_capacity) {

        data = (T*)std::malloc(capacity * sizeof(T));
        if (!data) {
            std::cerr << "Memory allocation failed!" << std::endl;
            std::exit(EXIT_FAILURE);
        }
        head = capacity / 2;
        tail = head;
    }

    ~ShiftToMiddleArray() {
        std::free(data);
    }

    void insert_head(const T& value) {
        if (head == 0) resize();
        data[--head] = value;
    }

    inline void push_front(const T& value) {
        insert_head(value);
    }

    void insert_tail(const T& value) {
        if (tail == capacity) resize();
        data[tail++] = value;
    }

    inline void push_back(const T& value) {
        insert_tail(value);
    }


    void insert(int at, const T& value) {
        int mid = (head + tail) / 2;
        if (at < mid && head > 0) {
            // Shift head left
            head--;
            for (int i = head; i < at; ++i) {
                data[i] = data[i + 1];
            }
            data[at] = value;
        } else if (tail < capacity) {
            // Shift tail right
            for (int i = tail; i > at; --i) {
                data[i] = data[i - 1];
            }
            data[at] = value;
            tail++;
        } else {
            // Resize if needed
            resize();
            insert(at, value);
        }
    }

    //Uncomment for safe behaviour
    void remove_head() {
        //if (head < tail) {
            ++head;
        //}
    }

    void remove_tail() {
        //if (tail > head) {
            --tail;
        //}
    }

    T get_head() const {
        if (head < tail) return data[head];
        throw std::out_of_range("Array is empty");
    }

    T get_tail() const {
        if (tail > head) return data[tail - 1];
        throw std::out_of_range("Array is empty");
    }

    inline void push(const T& value) {
        insert_tail(value);
    }

    inline void pop() {
        remove_head();
    }

    inline void pop_front() {
        remove_head();
    }

    inline void pop_back() {
        remove_tail();
    }

    inline T front() const {
        get_tail();
    }

    T& back() {

        if (head == tail) {
            throw std::out_of_range("ShiftToMiddleArray is empty!");
        }
        return data[tail - 1]; // Return the last valid element
    }

    const T& back() const {
        if (head == tail) {
            throw std::out_of_range("ShiftToMiddleArray is empty!");
        }
        return data[tail - 1]; // Const version
    }

    int size() const {
        return tail - head;
    }

    bool is_empty() const {
        return head == tail;
    }

    inline bool empty() const {
        return is_empty();
    }

    T& at(int index) {
        if (index < 0 || index >= size()) {
            throw std::out_of_range("Index out of range");
        }
        return data[(head + index) % capacity];
    }

    T& operator[](int index) {
        return data[head + index]; // No bounds checking
    }

    const T& at(int index) const {
        if (index < 0 || index >= size()) {
            throw std::out_of_range("Index out of range");
        }
        return data[(head + index) % capacity];
    }

    const T& operator[](int index) const {
        return data[head + index]; // No bounds checking
    }

void shrink_to_fit() {
    int size = tail - head;
    if (size == 0) {
        delete[] data;
        capacity = 8;
        data = new T[capacity];
        head = tail = capacity / 2;
        return;
    }

    // Allocate new minimal capacity (size + small buffer for head/tail operations)
    int new_capacity = size + 16; // 16 extra space for potential insertions
    T* new_data = new T[new_capacity];

    // Re-center data in the new array
    int new_head = (new_capacity - size) / 2;
    std::copy(data + head, data + tail, new_data + new_head);

    // Replace old data
    delete[] data;
    data = new_data;
    capacity = new_capacity;
    head = new_head;
    tail = new_head + size;
}

};
