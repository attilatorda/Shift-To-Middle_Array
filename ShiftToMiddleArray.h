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

#define BIAS_MULT 0.05f  // Toggle adaptive midpoint
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
    size_t head, tail, capacity_;
	float resize_multiplier;
#ifdef BIAS_MULT
	float bias;
#endif

    void resize_if_needed() {
		
		size_t new_capacity;
		
#ifdef ALLOW_SHRINKING
		if (size()  < capacity_ / 8 && capacity_ > 4) {  
			new_capacity = std::max({
				size()  * 2,
				static_cast<size_t>(4),
			});
		}
		else
#endif
		
		if (size()  > 2 && size() <  capacity_ / 2) {
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

        size_t new_head = (new_capacity - (tail - head)) / 2;

#ifdef BIAS_MULT
		// Apply dynamic biasing
		bool bias_is_negative = (bias < 0.0f);
		float abs_bias = std::abs(bias);
		size_t bias_offset = static_cast<size_t>(abs_bias * static_cast<double>(new_capacity));

		if (bias_is_negative) {
			// Handle negative bias: shift left
			if (bias_offset > new_head) {
				new_head = 0;
				bias += BIAS_MULT;
			} else {
				new_head -= bias_offset;
			}
		} else {
			// Handle non-negative bias: shift right
			if (new_head + size()  + bias_offset > new_capacity) {
				new_head = new_capacity - size() ;
				bias -= BIAS_MULT;
			} else {
				new_head += bias_offset;
			}
		}
#endif
		
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
		
		const size_t current_size = size();
		if (current_size == 0 || head == (capacity_ - current_size) / 2) return;

		T* new_head = data + (capacity_ - current_size) / 2;

		if constexpr (std::is_trivially_copyable_v<T>) {
			std::memmove(new_head, data + head, current_size * sizeof(T));
		} else {
			if (new_head < data + head) {  // Shift left
				std::move(data + head, data + tail, new_head);
				for (T* p = new_head + current_size; p < data + tail; ++p) p->~T();
			} else {  // Shift right
				std::move_backward(data + head, data + tail, new_head + current_size);
				for (T* p = data + head; p < new_head; ++p) p->~T();
			}
		}

		tail = (head = new_head - data) + current_size;
	}

public:
    ShiftToMiddleArray() : ShiftToMiddleArray(8) {}

    explicit ShiftToMiddleArray(size_t initial_capacity) 
        : capacity_(initial_capacity) 
    {
		if (initial_capacity == 0) {
			// throw std::invalid_argument("Initial capacity cannot be zero.");
			capacity_ = 1;
		}

        data = static_cast<T*>(std::malloc(capacity_ * sizeof(T)));
        head = tail = capacity_ / 2;
#ifdef BIAS_MULT	
		bias = 0.0f;
#endif
        
        if (!data) {
            throw std::bad_alloc();
        }
    }

    // Rule of Five

    ~ShiftToMiddleArray() {
        for (size_t i = 0; i < size(); ++i) {
            data[(head + i) % capacity_].~T();
        }
        std::free(data);
    }

	ShiftToMiddleArray(const ShiftToMiddleArray& other)
		:  head(other.head),
		  tail(other.tail),
		  capacity_(other.capacity_)
#ifdef BIAS_MULT
		  ,bias(other.bias)
#endif		  
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

	ShiftToMiddleArray(ShiftToMiddleArray&& other) noexcept
		: data(other.data),
		  head(other.head),
		  tail(other.tail),
		  capacity_(other.capacity_)
#ifdef BIAS_MULT
		  ,bias(other.bias)
#endif		  		  
	{
		other.data = nullptr;
		other.head = other.tail = other.capacity_ = 0;
	}

	ShiftToMiddleArray& operator=(const ShiftToMiddleArray& other) {
		if (this != &other) {
			ShiftToMiddleArray temp(other);  // Use copy constructor
			swap(*this, temp);              // Swap with temporary
		}
		return *this;
	}

	ShiftToMiddleArray& operator=(ShiftToMiddleArray&& other) noexcept {
		swap(*this, other);
		return *this;
	}

	bool operator==(const ShiftToMiddleArray& other) const {
		// Quick checks: size and capacity (if needed)
		if (size() != other.size()) return false;
		if (empty() && other.empty()) return true; // Both empty

		// Compare each element in the active range [head, tail)
		for (size_t i = 0; i < size(); ++i) {
			if (data[head + i] != other.data[other.head + i]) {
				return false;
			}
		}
		return true;
	}

	friend void swap(ShiftToMiddleArray& a, ShiftToMiddleArray& b) noexcept {
		using std::swap;
		swap(a.data, b.data);
		swap(a.head, b.head);
		swap(a.tail, b.tail);
		swap(a.capacity_, b.capacity_);
#ifdef BIAS_MULT	
		swap(a.bias, b.bias);
#endif
	}
	
	// Capacity observers

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
        if (head == 0) {
#ifdef BIAS_MULT				
			bias += BIAS_MULT;
#endif
			resize_if_needed();
		}
        data[--head] = value;
    }

    void push_back(const T& value) {
        if (tail == capacity_) {
#ifdef BIAS_MULT				
			bias -= BIAS_MULT;
#endif
			resize_if_needed();
		}
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

    // Iterator System
	
    template <bool Const>
    class IteratorBase {
        using ptr_t = std::conditional_t<Const, const T*, T*>;
        ptr_t ptr;
    public:
        explicit IteratorBase(ptr_t p) : ptr(p) {}
        using reference = std::conditional_t<Const, const T&, T&>;
        
        reference operator*() const { return *ptr; }
        IteratorBase& operator++() { ++ptr; return *this; }
        IteratorBase operator++(int) { auto tmp = *this; ++ptr; return tmp; }
        bool operator!=(const IteratorBase& other) const { return ptr != other.ptr; }
    };

    using iterator = IteratorBase<false>;
    using const_iterator = IteratorBase<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() { return iterator(data + head); }
    iterator end()   { return iterator(data + tail); }
    const_iterator begin() const { return const_iterator(data + head); }
    const_iterator end() const   { return const_iterator(data + tail); }
    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const   { return end(); }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend()   { return reverse_iterator(begin()); }
    const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }
    const_reverse_iterator crend() const   { return const_reverse_iterator(cbegin()); }

	// Other methods

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

	void serialize(std::ostream& os) const {
		os.write(reinterpret_cast<const char*>(&head), sizeof(size_t));
		size_t current_size = size(); // Store size to avoid recalculating
		os.write(reinterpret_cast<const char*>(&current_size), sizeof(size_t));
		os.write(reinterpret_cast<const char*>(data + head), current_size * sizeof(T));
	}

	bool deserialize(std::istream& is) {
		// Read head (starting index)
		if (!is.read(reinterpret_cast<char*>(&head), sizeof(size_t))) {
			return false;
		}

		// Read tail (end index) or size to reconstruct tail
		size_t serialized_size;
		if (!is.read(reinterpret_cast<char*>(&serialized_size), sizeof(size_t))) {
			return false;
		}

		// Read data into buffer
		is.read(reinterpret_cast<char*>(data + head), serialized_size * sizeof(T));
		if (is.fail()) {
			return false;
		}

		// Reconstruct tail
		tail = head + serialized_size;
		return true;
	}
};

template<typename T>
void swap(ShiftToMiddleArray<T>& lhs, ShiftToMiddleArray<T>& rhs) noexcept {
    lhs.swap(rhs);
}
