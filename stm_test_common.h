#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <deque>
#include <list>
#include <random>
#include <sstream>

#include "ShiftToMiddleArray.h"

template <typename T>
static void assert_same_as_deque(const ShiftToMiddleArray<T>& stm, const std::deque<T>& ref) {
    assert(stm.size() == ref.size());
    assert(stm.empty() == ref.empty());
    for (size_t i = 0; i < ref.size(); ++i) assert(stm[i] == ref[i]);
    if (!ref.empty()) {
        assert(stm.front() == ref.front());
        assert(stm.back() == ref.back());
    }
}

template <typename T>
static void assert_same_as_list(const ShiftToMiddleArray<T>& stm, const std::list<T>& ref) {
    assert(stm.size() == ref.size());
    assert(stm.empty() == ref.empty());
    size_t i = 0;
    for (const T& x : ref) assert(stm[i++] == x);
    if (!ref.empty()) {
        assert(stm.front() == ref.front());
        assert(stm.back() == ref.back());
    }
}

template <typename T>
static auto list_iter_at(std::list<T>& l, size_t idx) {
    auto it = l.begin();
    std::advance(it, static_cast<std::ptrdiff_t>(idx));
    return it;
}

static void test_basic_push_pop() {
    ShiftToMiddleArray<int> s;
    s.push_back(2); s.push_front(1); s.push_back(3);
    assert(s.size() == 3 && s.front() == 1 && s.back() == 3 && s[1] == 2);
    s.pop_front(); assert(s.front() == 2);
    s.pop_back(); assert(s.front() == 2 && s.back() == 2 && s.size() == 1);
    s.pop_back(); assert(s.empty());
    s.pop_back(); s.pop_front(); assert(s.empty());
}

static void test_insert_delete_at() {
    ShiftToMiddleArray<int> s; std::deque<int> d;
    for (int i = 0; i < 8; ++i) { s.push_back(i); d.push_back(i); }
    s.insert(0, 111); d.insert(d.begin(), 111);
    s.insert(3, 222); d.insert(d.begin() + 3, 222);
    s.insert(s.size(), 333); d.insert(d.end(), 333);
    assert_same_as_deque(s, d);
    s.delete_at(0); d.erase(d.begin());
    s.delete_at(2); d.erase(d.begin() + 2);
    s.delete_at(s.size() - 1); d.erase(d.end() - 1);
    assert_same_as_deque(s, d);
}

static void test_copy_move_equality() {
    ShiftToMiddleArray<int> a;
    for (int i = 0; i < 50; ++i) a.push_back(i * 3);
    ShiftToMiddleArray<int> b(a); assert(a == b);
    ShiftToMiddleArray<int> c; c = a; assert(c == a);
    ShiftToMiddleArray<int> d(std::move(c)); assert(d == a);
    ShiftToMiddleArray<int> e; e = std::move(d); assert(e == a);
}

static void test_serialize_roundtrip_small() {
    ShiftToMiddleArray<int> s;
    s.push_back(10); s.push_back(20); s.push_front(5);
    std::stringstream ss; s.serialize(ss);
    ShiftToMiddleArray<int> restored(16);
    assert(restored.deserialize(ss));
    assert(restored.size() == 3 && restored[0] == 5 && restored[1] == 10 && restored[2] == 20);
}

static void test_sanity_boundaries() {
    ShiftToMiddleArray<int> s(1);
    for (int i = 0; i < 1000; ++i) (i % 2 == 0) ? s.push_front(i) : s.push_back(i);
    assert(s.size() == 1000);
    for (int i = 0; i < 500; ++i) { s.pop_front(); s.pop_back(); }
    assert(s.empty());
}

static void test_insert_delete_exceptions() {
    ShiftToMiddleArray<int> s;
    bool thrown = false;
    try { s.insert(1, 42); } catch (const std::out_of_range&) { thrown = true; }
    assert(thrown);
}

static void differential_randomized(uint64_t seed, int ops) {
    std::mt19937_64 rng(seed);
    ShiftToMiddleArray<int> s;
    std::list<int> l;
    for (int i = 0; i < ops; ++i) {
        uint64_t r = rng();
        int value = static_cast<int>(r & 0x7fffffff);
        int op = static_cast<int>(r % 7);
        switch (op) {
            case 0: s.push_front(value); l.push_front(value); break;
            case 1: s.push_back(value); l.push_back(value); break;
            case 2: if (!l.empty()) { s.pop_front(); l.pop_front(); } break;
            case 3: if (!l.empty()) { s.pop_back(); l.pop_back(); } break;
            case 4: if (!l.empty()) { size_t idx = static_cast<size_t>(rng() % l.size()); s.delete_at(idx); l.erase(list_iter_at(l, idx)); } break;
            case 5: { size_t idx = l.empty() ? 0 : static_cast<size_t>(rng() % (l.size() + 1)); s.insert(idx, value); l.insert(list_iter_at(l, idx), value); break; }
            case 6: if (!l.empty()) { size_t idx = static_cast<size_t>(rng() % l.size()); assert(s[idx] == *list_iter_at(l, idx)); } break;
        }
        assert_same_as_list(s, l);
    }
}
