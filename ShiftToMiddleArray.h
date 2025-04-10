#pragma once

#include <cstdlib>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <cassert>  // For assert()
#include <type_traits> // For is_trivially_copyable

// Configuration: Define STM_BOUNDS_CHECK to enable bounds checking
// Comment this line out for unchecked (release) mode
//#define STM_BOUNDS_CHECK

#ifdef STM_BOUNDS_CHECK
  #define STM_ASSERT(cond, msg) assert((cond) && (msg))
#else
  #define STM_ASSERT(cond, msg) ((void)0)
#endif

template <typename T, float ResizeMult = 2.0f>
class ShiftToMiddleArray {

private:
    T* data;
    size_t  head, tail, capacity_;
	float resize_multiplier;

    void resize() {
        size_t new_capacity = static_cast<size_t>(capacity_ * ResizeMult);
        T* new_data = static_cast<T*>(std::malloc(new_capacity * sizeof(T)));
        
        if (!new_data) throw std::bad_alloc();

        size_t  new_head = (new_capacity - (tail - head)) / 2;

		//Copy data
		if constexpr (std::is_trivially_copyable_v<T>) {
			std::memcpy(new_data + new_head, data + head, (tail - head) * sizeof(T));
		} else {
			std::uninitialized_copy(data + head, data + tail, new_data + new_head);
		}

        std::free(data);
        data = new_data;
        tail = new_head + (tail - head);
        head = new_head;
        capacity_ = new_capacity;
    }

public:
    ShiftToMiddleArray() : ShiftToMiddleArray(8) {}

    explicit ShiftToMiddleArray(size_t initial_capacity) 
        : capacity_(initial_capacity) 
    {
        data = static_cast<T*>(std::malloc(capacity_ * sizeof(T)));
        head = tail = capacity_ / 2;
        
        if (!data) {
            throw std::bad_alloc();
        }
    }

    ~ShiftToMiddleArray() {
        // Call destructors for initialized elements
        for (size_t i = 0; i < size(); ++i) {
            data[(head + i) % capacity_].~T();
        }
        std::free(data);
    }

	// Copy constructor
	ShiftToMiddleArray(const ShiftToMiddleArray& other)
		: capacity_(other.capacity_),
		  head(other.head),
		  tail(other.tail)
	{
		if (other.capacity_ > 0) {
			data = static_cast<T*>(std::malloc(other.capacity_ * sizeof(T)));
			if (!data) throw std::bad_alloc();
			
			if constexpr (std::is_trivially_copyable_v<T>) {
				std::memcpy(data, other.data, other.capacity_ * sizeof(T));
			} else {
				try {
					std::uninitialized_copy(other.data + other.head, other.data + other.tail, data + head);
				} catch (...) {
					std::free(data);
					throw;
				}
			}
		} else {
			data = nullptr;
		}
	}

	// Move constructor (noexcept)
	ShiftToMiddleArray(ShiftToMiddleArray&& other) noexcept
		: data(other.data),
		  head(other.head),
		  tail(other.tail),
		  capacity_(other.capacity_)
	{
		other.data = nullptr;
		other.head = other.tail = other.capacity_ = 0;
	}

	// Copy assignment (strong exception safety)
	ShiftToMiddleArray& operator=(const ShiftToMiddleArray& other) {
		if (this != &other) {
			ShiftToMiddleArray temp(other);  // Use copy constructor
			swap(*this, temp);              // Swap with temporary
		}
		return *this;
	}

	// Move assignment (noexcept)
	ShiftToMiddleArray& operator=(ShiftToMiddleArray&& other) noexcept {
		swap(*this, other);
		return *this;
	}

	// Swap function (noexcept)
	friend void swap(ShiftToMiddleArray& a, ShiftToMiddleArray& b) noexcept {
		using std::swap;
		swap(a.data, b.data);
		swap(a.head, b.head);
		swap(a.tail, b.tail);
		swap(a.capacity_, b.capacity_);
	}

    size_t size() const noexcept { return tail - head; }
    bool empty() const noexcept { return head == tail; }
    size_t capacity() const noexcept { return capacity_; }

	// Accessors

    T& operator[](size_t  index) {
        STM_ASSERT(index >= 0 && index < size(), "Index out of range");
        return data[head + index];
    }

    const T& operator[](size_t  index) const {
        STM_ASSERT(index >= 0 && index < size(), "Index out of range");
        return data[head + index];
    }

    T& front() {
        STM_ASSERT(!empty(), "Array is empty");
        return data[head];
    }

    const T& front() const {
        STM_ASSERT(!empty(), "Array is empty");
        return data[head];
    }

    const T& get_head() const { return front(); }

    T& back() {
        STM_ASSERT(!empty(), "Array is empty");
        return data[tail - 1];
    }

    const T& back() const {
        STM_ASSERT(!empty(), "Array is empty");
        return data[tail - 1];
    }

    // Modifiers
	
    void push_front(const T& value) {
        if (head == 0) resize();
        data[--head] = value;
    }

    void push_back(const T& value) {
        if (tail == capacity_) resize();
        data[tail++] = value;
    }

    void push(const T& value) {
        push_back(value);
    }

    void insert_tail(const T& value) {
        push_back(value);
    }

	void pop_front() {
		remove_head();
	}

	void pop_back() {
		remove_tail();
	}

    void pop(const T& value) {
        pop_back(value);
    }
	
    void pop() {
        remove_head();
    }	

    void remove_head() {
        if (!empty()) ++head;
    }

    void remove_tail() {
        if (!empty()) --tail;
    }

    void insert(size_t  at, const T& value) {
        if (at < head || at > tail) {
            throw std::out_of_range("Insert position out of range");
        }

        size_t  mid = (head + tail) / 2;
        if (at < mid) {
            if (head == 0) resize();
            --head;
            std::memmove(data + head, data + head + 1, (at - head) * sizeof(T));
            data[at] = value;
        } else {
            if (tail == capacity_) resize();
            std::memmove(data + at + 1, data + at, (tail - at) * sizeof(T));
            data[at] = value;
            ++tail;
        }
    }

	// Helpers

    void shrink_to_fit() {
        size_t new_capacity = size();
        T* new_data = static_cast<T*>(std::malloc(new_capacity * sizeof(T)));
        if (!new_data) throw std::bad_alloc();

        std::memcpy(new_data, data + head, (tail - head) * sizeof(T));
        std::free(data);
        data = new_data;
        tail -= head;
        head = 0;
        capacity_ = new_capacity;
    }
};
