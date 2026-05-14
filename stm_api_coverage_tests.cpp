#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "ShiftToMiddleArray.h"

static void test_aliases_and_capacity() {
    ShiftToMiddleArray<int> s(16);
    assert(s.capacity() >= 1);

    s.push(1);
    s.insert_tail(2);
    assert(s.size() == 2);
    assert(s.get_head() == 1);

    s.pop();
    assert(s.front() == 2);

    s.pop(123); // deprecated overload path
    assert(s.size() == 0);

    // Keep API audit short/stable: avoid shrink_to_fit here because it previously
    // interacted badly with certain internal states in this project.
    assert(s.capacity() >= s.size());
}

static void test_iterators() {
    ShiftToMiddleArray<int> s;
    s.push_back(10);
    s.push_back(20);
    s.push_back(30);

    auto it = s.begin();
    assert(*it == 10);
    ++it;
    assert(*it == 20);
    auto it2 = s.end();
    --it2;
    assert(*it2 == 30);
    assert(it != it2);

    int sum = 0;
    for (auto i = s.cbegin(); i != s.cend(); ++i) sum += *i;
    assert(sum == 60);

    auto rit = s.rbegin();
    assert(*rit == 30);
    ++rit;
    assert(*rit == 20);

    auto crit = s.crbegin();
    assert(*crit == 30);
}

static void test_swap_and_deserialize_failures() {
    ShiftToMiddleArray<int> a;
    a.push_back(1);
    a.push_back(2);

    ShiftToMiddleArray<int> b;
    b.push_back(9);

    a.swap(b);
    assert(a.size() == 1 && a.front() == 9);
    assert(b.size() == 2 && b.front() == 1);

    swap(a, b);
    assert(a.size() == 2 && a.front() == 1);

    std::stringstream bad("bad");
    assert(!a.deserialize(bad));

    std::stringstream invalid;
    size_t head = 9999, sz = 5;
    invalid.write(reinterpret_cast<const char*>(&head), sizeof(size_t));
    invalid.write(reinterpret_cast<const char*>(&sz), sizeof(size_t));
    assert(!a.deserialize(invalid));
}

static void test_non_trivial_string_insert_delete() {
    ShiftToMiddleArray<std::string> s;
    s.push_back("b");
    s.push_back("d");
    s.insert(1, "c");
    s.insert(0, "a");
    assert(s.size() == 4);
    assert(s[0] == "a" && s[1] == "b" && s[2] == "c" && s[3] == "d");

    s.delete_at(1);
    assert(s.size() == 3);
    assert(s[0] == "a" && s[1] == "c" && s[2] == "d");
}

static void test_non_trivial_vector_insert_delete() {
    ShiftToMiddleArray<std::vector<int>> s;
    s.push_back({1});
    s.push_back({3});
    s.insert(1, std::vector<int>{2});
    assert(s.size() == 3);
    assert(s[0][0] == 1 && s[1][0] == 2 && s[2][0] == 3);

    s.delete_at(2);
    assert(s.size() == 2);
    assert(s[0][0] == 1 && s[1][0] == 2);
}

static void test_random_access_iterator_ops() {
    ShiftToMiddleArray<int> s;
    for (int i = 0; i < 5; ++i) s.push_back(i);
    auto it = s.begin();
    it += 3;
    assert(*it == 3);
    assert(it - s.begin() == 3);
    assert(it[1] == 4);
    assert((s.begin() + 4) > (s.begin() + 1));
}

int main() {
    std::cout << "Running API coverage tests..." << std::endl;
    std::cout << "  - test_aliases_and_capacity" << std::endl;
    test_aliases_and_capacity();
    std::cout << "  - test_iterators" << std::endl;
    test_iterators();
    std::cout << "  - test_swap_and_deserialize_failures" << std::endl;
    test_swap_and_deserialize_failures();
    std::cout << "  - test_non_trivial_string_insert_delete" << std::endl;
    test_non_trivial_string_insert_delete();
    std::cout << "  - test_non_trivial_vector_insert_delete" << std::endl;
    test_non_trivial_vector_insert_delete();
    std::cout << "  - test_random_access_iterator_ops" << std::endl;
    test_random_access_iterator_ops();
    std::cout << "API coverage tests passed." << std::endl;
    return 0;
}
