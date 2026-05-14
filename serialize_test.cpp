#include <cassert>   // For assertions
#include <sstream>   // For std::stringstream
#include <iostream>  // For debug output

#include "ShiftToMiddleArray.h"

// --- Tests ---
void testSerializeDeserialize() {
    ShiftToMiddleArray<int> q;
    q.push(10);
    q.push(20);

    std::stringstream ss;
    q.serialize(ss);           // Serialize
    ShiftToMiddleArray<int> restored;
    assert(restored.deserialize(ss)); // Deserialize
    assert(q == restored);
    std::cout << "PASS: Basic serialize/deserialize\n";
}

void testEmptyQueue() {
    ShiftToMiddleArray<int> q;
    std::stringstream ss;
    q.serialize(ss);
    ShiftToMiddleArray<int> restored;
    assert(restored.deserialize(ss));
    assert(restored.empty());
    std::cout << "PASS: Empty STM\n";
}

void testCorruptedStream() {
    ShiftToMiddleArray<int> q;
    q.push(1);
    std::stringstream ss("garbage_data"); // Invalid stream
    assert(!q.deserialize(ss)); // Should fail
    std::cout << "PASS: Corrupted stream handling\n";
}

int main() {
    testSerializeDeserialize();
    testEmptyQueue();
    testCorruptedStream();
    std::cout << "All tests passed!\n";
    return 0;
}
