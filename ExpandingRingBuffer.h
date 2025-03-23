#pragma once
#include <cstdlib>
#include <iostream>
#include <stdexcept>

template <typename T>
class ExpandingRingBuffer {
private:
    T* buffer;
    size_t head, tail, capacity;

    void resize() {
        size_t new_capacity = capacity * 2;
        T* new_buffer = (T*)std::malloc(new_capacity * sizeof(T));
        if (!new_buffer) {
            std::cerr << "Memory allocation failed!" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        // Copy elements to new buffer
        size_t size = (tail >= head) ? (tail - head) : (capacity - head + tail);
        for (size_t i = 0; i < size; ++i) {
            new_buffer[i] = buffer[(head + i) % capacity];
        }

        std::free(buffer);
        buffer = new_buffer;
        head = 0;
        tail = size;
        capacity = new_capacity;
    }

public:
    explicit ExpandingRingBuffer(size_t size = 8)
        : capacity(size), head(0), tail(0) {
        buffer = (T*)std::malloc(capacity * sizeof(T));
        if (!buffer) std::exit(EXIT_FAILURE);
    }

    ~ExpandingRingBuffer() {
        std::free(buffer);
    }

    void push(const T& item) {
        if ((tail + 1) % capacity == head) resize();
        buffer[tail] = item;
        tail = (tail + 1) % capacity;
    }

    void pop(T& item) {
        //if (head != tail) {
            item = buffer[head];
            head = (head + 1) % capacity;
        //}
    }

    T pop() {
        //if (head != tail) {
            T item = buffer[head];
            head = (head + 1) % capacity;
            return item;
        //}
    }

    // New methods to support deque functionality
    void push_front(const T& item) {
        if (head == 0) resize();
        head = (head - 1 + capacity) % capacity;
        buffer[head] = item;
    }

    inline void push_back(const T& item) {
        push(item);
    }

    inline T pop_front() {
        return pop();
    }

    bool pop_back(T& item) {
        //if (head != tail) {
            tail = (tail - 1 + capacity) % capacity;
            item = buffer[tail];
            return true;
        //}
        //return false;
    }

    T pop_back() {
        //if (head != tail) {
            tail = (tail - 1 + capacity) % capacity;
            return buffer[tail]; // Return the popped item
        //}
        //throw std::out_of_range("Ring buffer is empty!"); // Ensure safe behavior
    }

    T front() const {
        //if (!empty()) {
            return buffer[head];
        //}
        //throw std::out_of_range("Deque is empty");
    }

    T back() const {
        //if (!empty()) {
            return buffer[(tail - 1 + capacity) % capacity];
        //}
        //throw std::out_of_range("Deque is empty");
    }

    T& operator[](size_t index) {
        //if (index < size()) {
            return buffer[(head + index) % capacity]; // Wrap around using modulo
        //}
        //throw std::out_of_range("Index is out of range");
    }

    const T& operator[](size_t index) const {
        //if (index < size()) {
            return buffer[(head + index) % capacity]; // Wrap around using modulo
        //}
        //throw std::out_of_range("Index is out of range");
    }

    size_t size() const { return (tail + capacity - head) % capacity; }
    bool empty() const { return head == tail; }
};
