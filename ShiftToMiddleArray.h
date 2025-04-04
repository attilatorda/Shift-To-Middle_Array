#pragma once

#include <cstdlib>
#include <cstring>
#include <memory>
#include <stdexcept>

template <typename T>
class ShiftToMiddleArray {
private:
    T* data;
    int head, tail, capacity;

    void resize() {
        int new_capacity = capacity ? capacity * 2 : 4;
        T* new_data = static_cast<T*>(std::malloc(new_capacity * sizeof(T)));
        if (!new_data) throw std::bad_alloc();

        int new_head = (new_capacity - (tail - head)) / 2;
		
		if constexpr (std::is_trivially_copyable_v<T>) {
			std::memcpy(new_data + new_head, data + head, (tail - head) * sizeof(T));
		} else {
			std::uninitialized_copy(data + head, data + tail, new_data + new_head);
		}

        std::free(data);
        data = new_data;
        tail = new_head + (tail - head);
        head = new_head;
        capacity = new_capacity;
    }

public:
    // Default constructor - creates buffer with default capacity
    ShiftToMiddleArray() : ShiftToMiddleArray(8) {}
    
    // Constructor with specific capacity
    explicit ShiftToMiddleArray(size_t initial_capacity) 
        : capacity(initial_capacity) 
    {
        data = static_cast<T*>(std::malloc(capacity * sizeof(T)));
        head = tail = capacity / 2;  // Start in the middle
        
        if (!data) {
            throw std::bad_alloc();
        }
    }
    
    ~ShiftToMiddleArray() { 
        std::free(data); 
    }


    int size() const { return tail - head; }

    bool empty() const { return head == tail; }

    T& operator[](int index) {
        if (index < 0 || index >= size()) {
            throw std::out_of_range("Index out of range");
        }
        return data[head + index];
    }

    const T& operator[](int index) const {
        if (index < 0 || index >= size()) {
            throw std::out_of_range("Index out of range");
        }
        return data[head + index];
    }

    T& front() {
        if (empty()) throw std::out_of_range("Array is empty");
        return data[head];
    }

    const T& front() const {
        if (empty()) throw std::out_of_range("Array is empty");
        return data[head];
    }

	inline const T& get_head() const {
		return front();  // Just calls your existing front() method
	}

    T& back() {
        if (empty()) throw std::out_of_range("Array is empty");
        return data[tail - 1];
    }

    const T& back() const {
        if (empty()) throw std::out_of_range("Array is empty");
        return data[tail - 1];
    }

    void push_front(const T& value) {
        if (head == 0) resize();
        data[--head] = value;
    }

    void push_back(const T& value) {
        if (tail == capacity) resize();
        data[tail++] = value;
    }

    inline void push(const T& value) {
        push_back(value);
    }

    inline void insert_tail(const T& value) {
        push_back(value);
    }

	inline void pop_front() {
		remove_head();
	}

	inline void pop_back() {
		remove_tail();
	}

    inline void pop(const T& value) {
        pop_back(value);
    }
	
    inline void pop() {
        remove_head();
    }	

    void remove_head() {
        if (!empty()) ++head;
    }

    void remove_tail() {
        if (!empty()) --tail;
    }

    void insert(int at, const T& value) {
        if (at < head || at > tail) {
            throw std::out_of_range("Insert position out of range");
        }

        int mid = (head + tail) / 2;
        if (at < mid) {
            if (head == 0) resize();
            --head;
            std::memmove(data + head, data + head + 1, (at - head) * sizeof(T));
            data[at] = value;
        } else {
            if (tail == capacity) resize();
            std::memmove(data + at + 1, data + at, (tail - at) * sizeof(T));
            data[at] = value;
            ++tail;
        }
    }

    void shrink_to_fit() {
        int new_capacity = size();
        T* new_data = static_cast<T*>(std::malloc(new_capacity * sizeof(T)));
        if (!new_data) throw std::bad_alloc();

        std::memcpy(new_data, data + head, (tail - head) * sizeof(T));
        std::free(data);
        data = new_data;
        tail -= head;
        head = 0;
        capacity = new_capacity;
    }
};
