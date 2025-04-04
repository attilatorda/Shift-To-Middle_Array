#include "ShiftToMiddleArray.h"

#include <boost/circular_buffer.hpp>
#include <cstdint>
#include <deque>
#include <iostream>
#include <list>
#include <random>

[[nodiscard]] constexpr uint64_t next_hash(const uint64_t x) noexcept { return x ^ ((x >> 17) | (x << (64 - 17))); }

std::mt19937_64 get_random(uint64_t seed) {
  std::seed_seq s{seed};
  std::mt19937_64 r{s};
  return r;
}

template <typename T> struct expanding_circular_buffer : public boost::circular_buffer<T> {
  void push_back(T x) {
    if (boost::circular_buffer<T>::full())
      boost::circular_buffer<T>::set_capacity(
          boost::circular_buffer<T>::capacity() ? boost::circular_buffer<T>::capacity() << 1 : 4);
    boost::circular_buffer<T>::push_back(x);
  }
  void push_front(T x) {
    if (boost::circular_buffer<T>::full())
      boost::circular_buffer<T>::set_capacity(
          boost::circular_buffer<T>::capacity() ? boost::circular_buffer<T>::capacity() << 1 : 4);
    boost::circular_buffer<T>::push_front(x);
  }
};

template <typename T> struct mid_iterator {
  const ShiftToMiddleArray<T> &c;
  size_t i;
  constexpr mid_iterator(const ShiftToMiddleArray<T> &c, bool b = true)
      : c{c}, i{b ? static_cast<size_t>(0) : c.size()} {}

  [[nodiscard]] constexpr bool operator!=(const mid_iterator &o) const noexcept { return &c != &o.c || i != o.i; }
  constexpr mid_iterator &operator++() noexcept {
    ++i;
    return *this;
  }
  [[nodiscard]] constexpr T operator*() const noexcept { return c[i]; }
};

template <typename T> mid_iterator<T> begin(const ShiftToMiddleArray<T> &c) { return mid_iterator{c, true}; }

template <typename T> mid_iterator<T> end(const ShiftToMiddleArray<T> &c) { return mid_iterator{c, false}; }

template <typename T> uint64_t test(uint64_t seed, uint64_t count) {
  T container;
  uint64_t h{0};
  auto r = get_random(seed);
  while (count--)
    if (const auto n = r(); (n & 0x80) && !container.empty()) {
      h = next_hash(h ^ container.front());
      container.pop_front();
    } else if ((n & 0x40) && !container.empty()) {
      h = next_hash(h ^ container.back());
      container.pop_back();
    } else if (n & 0x20)
      container.push_front(n);
    else if (n & 0x10)
      container.push_back(n);
    else
      for (uint64_t x : container)
        h = next_hash(h ^ x);

  return h;
}

int main() {
  static constexpr size_t size = 20;
  std::cout << "Making tests" << '\n';
  
  for (int i = 0; i < 10; i++) {
    std::cout << "deque   " << test<std::deque<uint32_t>>(i, size) << '\n';
    std::cout << "list    " << test<std::list<uint32_t>>(i, size) << '\n';
    std::cout << "circ    " << test<expanding_circular_buffer<uint32_t>>(i, size) << '\n';
    std::cout << "shift   " << test<ShiftToMiddleArray<uint32_t>>(i, size) << '\n' << '\n';
  }

  return 0;
}
