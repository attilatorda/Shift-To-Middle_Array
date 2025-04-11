#pragma once

#include <cstdlib>      // std::malloc, std::free, std::size_t
#include <cstring>      // std::memcpy, std::memmove
#include <memory>       // std::uninitialized_copy, std::uninitialized_move, std::addressof
#include <stdexcept>    // std::out_of_range, std::bad_alloc, std::logic_error
#include <cassert>      // assert()
#include <type_traits>  // std::is_trivially_copyable_v, etc.
#include <algorithm>    // std::max, std::min, std::clamp, std::move, std::move_backward, std::swap
#include <utility>      // std::swap (used via <algorithm>), std::forward
#include <iterator>     // std::random_access_iterator_tag, std::ptrdiff_t, std::reverse_iterator
#include <ostream>      // std::ostream, std::istream

//#define ALLOW_SHRINKING  // Toggle dynamic downsizing
#define STM_BOUNDS_CHECK  // Toggle this for bounds checking

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

    void resize_if_needed() {
		
		size_t new_capacity;
		
		#ifdef ALLOW_SHRINKING
		if (size() < capacity_ / 8 && capacity_ > 4) {  
			new_capacity = std::max({
				size() * 2,
				static_cast<size_t>(4),
			});
		}
		else
		#endif
		
		if (size() > 2 && size() <  capacity_ / 2) {
			shift_to_middle();
			return;
		}
		
		else {
			new_capacity = static_cast<size_t>(capacity_ * ResizeMult);		
		}
		
		resize(new_capacity);		
    }
	
	void resize(size_t new_capacity) {
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
	
	#ifdef ALLOW_SHRINKING
	void shrink_if_needed() {
		if(size() < capacity_ / 8 && capacity_ > 4) {
			resize(std::max({
				size() * 2,
				static_cast<size_t>(4),
			}));
		}
	}
	#endif
		
	void shift_to_middle() {
		size_t current_size = size(); // Calculate size *before* modifications
		size_t ideal_head;

		ideal_head = (capacity_ > current_size) ? (capacity_ - current_size) / 2 : 0;
		ideal_head = std::clamp(ideal_head, size_t(0), (capacity_ >= current_size) ? capacity_ - current_size : 0);
		
		if (head == ideal_head) return; // Already centered

		T* source_begin = data + head;
		T* source_end = data + tail;
		T* dest_begin = data + ideal_head; // Target start address for the block

		if constexpr (std::is_trivially_copyable_v<T>) {
			// Trivial types: memmove is safe and efficient for overlap
			std::memmove(dest_begin, source_begin, current_size * sizeof(T));
		} else {
			// Non-trivial types: Use std::move or std::move_backward
			if (ideal_head < head) { // Shifting left: Moving data to an earlier memory address
				// Use std::move: Copies [source_begin, source_end) to [dest_begin, ...)
				std::move(source_begin, source_end, dest_begin);
				// Destroy the objects left behind at the *end* of the original range
				// Range is [dest_begin + current_size, source_end)
				destroy_range(dest_begin + current_size, source_end);
			} else { // Shifting right: Moving data to a later memory address
				// Use std::move_backward: Copies [source_begin, source_end) to range ending at dest_begin + current_size
				std::move_backward(source_begin, source_end, dest_begin + current_size);
				// Destroy the objects left behind at the *beginning* of the original range
				// Range is [source_begin, dest_begin)
				destroy_range(source_begin, dest_begin);
			}
		}

		// Update indices safely using pre-calculated size
		head = ideal_head;
		tail = head + current_size; // Use current_size calculated before the shift
		return; // Indicate that a shift occurred
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
        if (head == 0) resize_if_needed();
        data[--head] = value;
    }

    void push_back(const T& value) {
        if (tail == capacity_) resize_if_needed();
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
		#ifdef ALLOW_SHRINKING
		shrink_if_needed();
		#endif		
    }

    void remove_tail() {
        if (!empty()) --tail;
		#ifdef ALLOW_SHRINKING
		shrink_if_needed();
		#endif		
    }

    void insert(size_t  at, const T& value) {
        if (at < head || at > tail) {
            throw std::out_of_range("Insert position out of range");
        }

        size_t  mid = (head + tail) / 2;
        if (at < mid) {
            if (head == 0) resize_if_needed();
            --head;
            std::memmove(data + head, data + head + 1, (at - head) * sizeof(T));
            data[at] = value;
        } else {
            if (tail == capacity_) resize_if_needed();
            std::memmove(data + at + 1, data + at, (tail - at) * sizeof(T));
            data[at] = value;
            ++tail;
        }
    }

	void delete_at(size_t index) {
		STM_ASSERT(index < size(), "ShiftToMiddleArray::delete_at - index " + std::to_string(index) + 
			" out of range (size=" + std::to_string(size()) + ")");
		
		size_t absolute_pos = head + index;
		bool closer_to_head = (index < size() / 2);
		
		// Destroy target element
		if constexpr (!std::is_trivially_destructible_v<T>) {
			data[absolute_pos].~T();
		}
		
		// Shift elements (direction-aware)
		if (closer_to_head) {
			if constexpr (std::is_trivially_copyable_v<T>) {
				std::memmove(data + head + 1, data + head, index * sizeof(T));
			} else {
				for (size_t i = absolute_pos; i > head; --i) {
					new (&data[i]) T(std::move(data[i - 1]));
					data[i - 1].~T();
				}
			}
			++head;
		} else {
			if constexpr (std::is_trivially_copyable_v<T>) {
				std::memmove(data + absolute_pos, data + absolute_pos + 1, 
							(tail - absolute_pos - 1) * sizeof(T));
			} else {
				for (size_t i = absolute_pos; i < tail - 1; ++i) {
					new (&data[i]) T(std::move(data[i + 1]));
					data[i + 1].~T();
				}
			}
			--tail;
		}
		
		#ifdef ALLOW_SHRINKING
		shrink_if_needed();
		#endif
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
