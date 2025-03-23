#pragma once
#include <cstdlib>
#include <iostream>
#include "ShiftToMiddleArray.h"

template <typename T>
class ShiftToMiddleArray {
private:
    T* data;
    int head, tail, capacity;

    void resize() {
        int new_capacity = capacity * 2;
        T* new_data = (T*)std::malloc(new_capacity * sizeof(T));
        if (!new_data) {
            std::cerr << "Memory allocation failed!" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        int new_head = (new_capacity - (tail - head)) / 2;
        for (int i = head, j = new_head; i < tail; ++i, ++j) {
            new_data[j] = data[i];
        }

        std::free(data);
        data = new_data;
        tail = new_head + (tail - head);
        head = new_head;
        capacity = new_capacity;
    }

public:
    ShiftToMiddleArray(int initial_capacity = 16)
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

    void insert_tail(const T& value) {
        if (tail == capacity) resize();
        data[tail++] = value;
    }

    void remove_head() {
        if (head < tail) {
            ++head;
        }
    }

    void remove_tail() {
        if (tail > head) {
            --tail;
        }
    }

    T get_head() const {
        if (head < tail) return data[head];
        throw std::out_of_range("Array is empty");
    }

    T get_tail() const {
        if (tail > head) return data[tail - 1];
        throw std::out_of_range("Array is empty");
    }

    bool is_empty() const {
        return head == tail;
    }
};
