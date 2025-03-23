import gnu.trove.iterator.TIntIterator;
import gnu.trove.list.TIntList;
import gnu.trove.list.array.TIntArrayList;
import gnu.trove.list.linked.TIntLinkedList;
import java.io.FileWriter;
import java.io.IOException;
import java.util.*;
import java.util.concurrent.ThreadLocalRandom;
import gnu.trove.procedure.TIntProcedure;
import gnu.trove.function.TIntFunction;
import gnu.trove.TIntCollection;

public class ShiftToMiddleArrayBenchmarkTrove {

    public static void main(String[] args) {
        runBenchmark(10000);
    }

    public static void runBenchmark(int operations) {
        int[] testSizes = {10, 50, 100, 1000, 5000};
        int runs = 8; // Number of benchmark runs to average

        try (FileWriter writer = new FileWriter("benchmark_results_list_trove.csv")) {
            writer.write("Size,Type,Time\n");

            for (int size : testSizes) {
                System.out.println("Test size: " + size);

                double arrayListTotal = 0.0;
                double linkedListTotal = 0.0;
                double stmTotal = 0.0;

                for (int i = 0; i < runs; i++) {
                    arrayListTotal += benchmarkRandomOperations(new TIntArrayList(10), size, operations);
                    linkedListTotal += benchmarkRandomOperations(new TIntLinkedList(), size, operations);
                    stmTotal += benchmarkRandomOperations(new ShiftToMiddleArray(10), size, operations);
                }

                double arrayListAvg = arrayListTotal / runs;
                double linkedListAvg = linkedListTotal / runs;
                double stmAvg = stmTotal / runs;

                System.out.println("Benchmarking TIntArrayList...");
                System.out.println("TIntArrayList (avg over " + runs + " runs): " + arrayListAvg + " ms");

                System.out.println("Benchmarking TIntLinkedList...");
                System.out.println("TIntLinkedList (avg over " + runs + " runs): " + linkedListAvg + " ms");

                System.out.println("Benchmarking ShiftToMiddleArray...");
                System.out.println("ShiftToMiddleArray (avg over " + runs + " runs): " + stmAvg + " ms");

                // Find the best time (lowest)
                double bestTime = Math.min(arrayListAvg, linkedListAvg);
                double stmSpeedup = ((bestTime - stmAvg) / bestTime) * 100;

                System.out.printf("ShiftToMiddleArray was %.2f%% %s than the best alternative.\n\n",
                        Math.abs(stmSpeedup), (stmSpeedup < 0 ? "slower" : "faster"));

                // Write results to CSV
                writer.write(size + ",TIntArrayList," + arrayListAvg + "\n");
                writer.write(size + ",TIntLinkedList," + linkedListAvg + "\n");
                writer.write(size + ",ShiftToMiddleArray," + stmAvg + "\n");
            }

            System.out.println("Results saved to benchmark_results_list_trove.csv");

        } catch (IOException e) {
            e.printStackTrace();
        }
    }


    public static double benchmarkRandomOperations(TIntList list, int size, int operations) {
        ThreadLocalRandom rng = ThreadLocalRandom.current();
        int[] opDist = {30, 30, 30, 10}; // 10% chance for spike
        boolean spikeMode = false;
        int storedValue = 0;

        long start = System.nanoTime();

        for (int i = 0; i < 10; ++i) { // 10 iterations
            // Initial insertions
            for (int j = 0; j < size; ++j) {
                list.add(j);
            }

            // Mixed random operations
            for (int j = 0; j < operations; ++j) {
                if (list.isEmpty()) continue;
                int index = rng.nextInt(list.size());

                int op = rng.nextInt(100);
                if (op < opDist[0]) { // Insert at random position
                    if (index < list.size()) list.set(index, j);
                    else list.add(j);
                } else if (op < opDist[0] + opDist[1]) { // Remove if not empty
                    if (index < list.size()) list.removeAt(index);
                } else if (op < opDist[0] + opDist[1] + opDist[2]) { // Read element
                    if (index < list.size()) {
                        storedValue = list.get(index);
                    }
                } else { // Spike event: randomly remove/add 10% of elements
                    int spikeSize = list.size() / 10;
                    for (int k = 0; k < spikeSize; ++k) {
                        int spikeIndex = rng.nextInt(list.size());

                        if (spikeMode && !list.isEmpty()) {
                            list.removeAt(spikeIndex);
                        } else {
                            list.insert(spikeIndex, k);
                        }
                    }
                    spikeMode = !spikeMode; // Alternate spike behavior
                }
            }
        }

        long end = System.nanoTime();
        return (end - start) / 1_000_000.0; // Convert to milliseconds
    }

    static class ShiftToMiddleArray implements TIntList {
        private int[] data;
        private int head, tail, capacity;

        public ShiftToMiddleArray(int initialCapacity) {
            this.capacity = initialCapacity;
            this.data = new int[capacity];
            this.head = capacity / 2;
            this.tail = head;
        }

        public ShiftToMiddleArray() {
            this(16);
        }

        private void resize() {
            int size = tail - head;

			/*
            if (size < capacity - 2) {
                shift(size);
                return;
            }*/

            int newCapacity = capacity * 2;
            int[] newData = new int[newCapacity];
            int newHead = (newCapacity - size) / 2;

            System.arraycopy(data, head, newData, newHead, size);

            data = newData;
            tail = newHead + size;
            head = newHead;
            capacity = newCapacity;
        }

        private void shift(int size) {
            int newHead = (capacity - size) / 2;
            int newTail = newHead + size;

            System.arraycopy(data, head, data, newHead, size);

            head = newHead;
            tail = newTail;
        }

        @Override
        public int size() {
            return tail - head;
        }

        @Override
        public boolean isEmpty() {
            return head == tail;
        }

        @Override
        public boolean contains(int value) {
            for (int i = head; i < tail; i++) {
                if (data[i] == value) return true;
            }
            return false;
        }

        @Override
        public int get(int index) {
            if (index < 0 || index >= size()) throw new IndexOutOfBoundsException();
            return data[head + index];
        }

        @Override
        public int set(int index, int value) {
            if (index < 0 || index >= size()) throw new IndexOutOfBoundsException();
            int oldValue = data[head + index];
            data[head + index] = value;
            return oldValue;
        }

        @Override
        public boolean add(int value) {
            if (tail == capacity) resize();
            data[tail++] = value;
            return true;
        }

        public void add(int index, int value) {
            if (index < 0 || index > size()) throw new IndexOutOfBoundsException();

            int mid = (head + tail) / 2;
            if (index < mid && head > 0) {
                // Shift head left
                head--;
                for (int i = head; i < index; i++) {
                    data[i] = data[i + 1];
                }
                data[index] = value;
            } else if (tail < capacity) {
                // Shift tail right
                for (int i = tail; i > index; i--) {
                    data[i] = data[i - 1];
                }
                data[index] = value;
                tail++;
            } else {
                // Resize if needed
                resize();
                add(index, value);
            }
        }

        @Override
        public int removeAt(int index) {
            if (index < 0 || index >= size()) throw new IndexOutOfBoundsException();
            int oldValue = data[head + index];
            for (int i = head + index; i < tail - 1; i++) {
                data[i] = data[i + 1];
            }
            tail--;
            return oldValue;
        }

        @Override
        public void clear() {
            head = tail = capacity / 2;
        }

        @Override
        public TIntIterator iterator() {
            return new TIntIterator() {
                private int current = head;

                @Override
                public boolean hasNext() {
                    return current < tail;
                }

                @Override
                public int next() {
                    if (!hasNext()) throw new NoSuchElementException();
                    return data[current++];
                }

                @Override
                public void remove() {
                    throw new UnsupportedOperationException();
                }
            };
        }

        @Override
        public int[] toArray() {
            int[] array = new int[size()];
            System.arraycopy(data, head, array, 0, size());
            return array;
        }

        @Override
        public boolean remove(int value) {
            for (int i = head; i < tail; i++) {
                if (data[i] == value) {
                    removeAt(i - head);
                    return true;
                }
            }
            return false;
        }

        @Override
        public boolean addAll(Collection<? extends Integer> collection) {
            throw new UnsupportedOperationException();
        }

        public boolean addAll(int index, Collection<? extends Integer> collection) {
            throw new UnsupportedOperationException();
        }

        @Override
        public boolean removeAll(Collection<?> collection) {
            throw new UnsupportedOperationException();
        }

        @Override
        public boolean retainAll(Collection<?> collection) {
            throw new UnsupportedOperationException();
        }

        @Override
        public int indexOf(int value) {
            for (int i = head; i < tail; i++) {
                if (data[i] == value) return i - head;
            }
            return -1;
        }

        @Override
        public int lastIndexOf(int value) {
            for (int i = tail - 1; i >= head; i--) {
                if (data[i] == value) return i - head;
            }
            return -1;
        }

        @Override
        public int sum() {
            int sum = 0;
            for (int i = head; i < tail; i++) {
                sum += data[i];
            }
            return sum;
        }

        @Override
        public int min() {
            if (isEmpty()) throw new NoSuchElementException();
            int min = data[head];
            for (int i = head + 1; i < tail; i++) {
                if (data[i] < min) min = data[i];
            }
            return min;
        }

        @Override
        public int max() {
            if (isEmpty()) throw new NoSuchElementException();
            int max = data[head];
            for (int i = head + 1; i < tail; i++) {
                if (data[i] > max) max = data[i];
            }
            return max;
        }

    @Override
    public TIntList inverseGrep(TIntProcedure procedure) {
        ShiftToMiddleArray result = new ShiftToMiddleArray();
        for (int i = head; i < tail; i++) {
            if (!procedure.execute(data[i])) {
                result.add(data[i]);
            }
        }
        return result;
    }
	
    @Override
    public TIntList grep(TIntProcedure procedure) {
        ShiftToMiddleArray result = new ShiftToMiddleArray();
        for (int i = head; i < tail; i++) {
            if (procedure.execute(data[i])) {
                result.add(data[i]);
            }
        }
        return result;
    }

    @Override
    public boolean forEach(TIntProcedure procedure) {
        for (int i = head; i < tail; i++) {
            if (!procedure.execute(data[i])) {
                return false;
            }
        }
        return true;
    }

    @Override
    public int[] toArray(int[] dest) {
        if (dest.length < size()) {
            dest = new int[size()];
        }
        System.arraycopy(data, head, dest, 0, size());
        return dest;
    }

    @Override
    public void fill(int value) {
        Arrays.fill(data, head, tail, value);
    }

    @Override
    public int binarySearch(int value) {
        int[] array = toArray();
        return Arrays.binarySearch(array, value);
    }

    @Override
    public void sort() {
        Arrays.sort(data, head, tail);
    }
	
	@Override
public void sort(int fromIndex, int toIndex) {
    if (fromIndex < 0 || toIndex > size() || fromIndex > toIndex) {
        throw new IndexOutOfBoundsException();
    }
    Arrays.sort(data, head + fromIndex, head + toIndex);
}

	
@Override
public int lastIndexOf(int value, int end) {
    if (end < 0 || end > size()) {
        throw new IndexOutOfBoundsException();
    }
    for (int i = head + end - 1; i >= head; i--) {
        if (data[i] == value) {
            return i - head;
        }
    }
    return -1;
}

@Override
public int indexOf(int value, int start) {
    if (start < 0 || start > size()) {
        throw new IndexOutOfBoundsException();
    }
    for (int i = head + start; i < tail; i++) {
        if (data[i] == value) {
            return i - head;
        }
    }
    return -1;
}	

@Override
public int binarySearch(int fromIndex, int toIndex, int value) {
    if (fromIndex < 0 || toIndex > size() || fromIndex > toIndex) {
        throw new IndexOutOfBoundsException();
    }

    // Create a temporary array for the specified range
    int[] tempArray = new int[toIndex - fromIndex];
    System.arraycopy(data, head + fromIndex, tempArray, 0, toIndex - fromIndex);

    // Perform binary search on the temporary array
    return Arrays.binarySearch(tempArray, value);
}

@Override
public void fill(int fromIndex, int toIndex, int value) {
    if (fromIndex < 0 || toIndex > size() || fromIndex > toIndex) {
        throw new IndexOutOfBoundsException();
    }

    // Fill the specified range with the value
    Arrays.fill(data, head + fromIndex, head + toIndex, value);
}

@Override
public int replace(int oldValue, int newValue) {
    int count = 0;
    for (int i = head; i < tail; i++) {
        if (data[i] == oldValue) {
            data[i] = newValue;
            count++;
        }
    }
    return count;
}

@Override
public void reverse() {
    for (int i = head, j = tail - 1; i < j; i++, j--) {
        int temp = data[i];
        data[i] = data[j];
        data[j] = temp;
    }
}

@Override
public int[] toArray(int[] dest, int offset, int length) {
    if (length > size()) {
        throw new IllegalArgumentException("Length exceeds list size");
    }
    System.arraycopy(data, head, dest, offset, length);
    return dest;
}

@Override
public boolean forEachDescending(TIntProcedure procedure) {
    for (int i = tail - 1; i >= head; i--) {
        if (!procedure.execute(data[i])) {
            return false;
        }
    }
    return true;
}

@Override
public int[] toArray(int[] dest, int sourceOffset, int destOffset, int length) {
    if (sourceOffset < 0 || destOffset < 0 || length < 0 ||
        sourceOffset + length > size() || destOffset + length > dest.length) {
        throw new IndexOutOfBoundsException();
    }

    // Copy elements from the list to the destination array
    System.arraycopy(data, head + sourceOffset, dest, destOffset, length);

    // Return the destination array
    return dest;
}

@Override
public int[] toArray(int offset, int length) {
    if (offset < 0 || length < 0 || offset + length > size()) {
        throw new IndexOutOfBoundsException();
    }

    // Create a new array for the specified range
    int[] result = new int[length];
    System.arraycopy(data, head + offset, result, 0, length);

    return result;
}

@Override
public void shuffle(Random random) {
    for (int i = tail - 1; i > head; i--) {
        int j = head + random.nextInt(i - head + 1);
        int temp = data[i];
        data[i] = data[j];
        data[j] = temp;
    }
}

@Override
public void reverse(int fromIndex, int toIndex) {
    if (fromIndex < 0 || toIndex > size() || fromIndex > toIndex) {
        throw new IndexOutOfBoundsException();
    }

    // Reverse the specified range
    for (int i = head + fromIndex, j = head + toIndex - 1; i < j; i++, j--) {
        int temp = data[i];
        data[i] = data[j];
        data[j] = temp;
    }
}

@Override
public void transformValues(TIntFunction function) {
    for (int i = head; i < tail; i++) {
        data[i] = function.execute(data[i]);
    }
}

@Override
public void remove(int fromIndex, int toIndex) {
    if (fromIndex < 0 || toIndex > size() || fromIndex > toIndex) {
        throw new IndexOutOfBoundsException();
    }

    // Calculate the number of elements to remove
    int numToRemove = toIndex - fromIndex;

    // Shift elements after the range to the left
    System.arraycopy(data, head + toIndex, data, head + fromIndex, tail - (head + toIndex));

    // Update the tail pointer
    tail -= numToRemove;
}

@Override
public void set(int offset, int[] values, int valuesOffset, int length) {
    if (offset < 0 || valuesOffset < 0 || length < 0 ||
        offset + length > size() || valuesOffset + length > values.length) {
        throw new IndexOutOfBoundsException();
    }

    // Copy values from the provided array into the list
    System.arraycopy(values, valuesOffset, data, head + offset, length);
}

@Override
public void set(int offset, int[] values) {
    if (offset < 0 || offset + values.length > size()) {
        throw new IndexOutOfBoundsException();
    }

    // Copy values from the provided array into the list
    System.arraycopy(values, 0, data, head + offset, values.length);
}

@Override
public void insert(int offset, int[] values) {
    if (offset < 0 || offset > size()) {
        throw new IndexOutOfBoundsException();
    }

    // Ensure there is enough capacity for the new elements
    while (tail + values.length > capacity) {
        resize();
    }

    // Shift elements to the right to make space for the new elements
    System.arraycopy(data, head + offset, data, head + offset + values.length, tail - (head + offset));

    // Copy values from the provided array into the list
    System.arraycopy(values, 0, data, head + offset, values.length);

    // Update the tail pointer
    tail += values.length;
}

@Override
public void insert(int offset, int[] values, int valuesOffset, int length) {
    if (offset < 0 || valuesOffset < 0 || length < 0 ||
        offset > size() || valuesOffset + length > values.length) {
        throw new IndexOutOfBoundsException();
    }

    // Ensure there is enough capacity for the new elements
    while (tail + length > capacity) {
        resize();
    }

    // Shift elements to the right to make space for the new elements
    System.arraycopy(data, head + offset, data, head + offset + length, tail - (head + offset));

    // Copy values from the provided array into the list
    System.arraycopy(values, valuesOffset, data, head + offset, length);

    // Update the tail pointer
    tail += length;
}

@Override
public void insert(int offset, int value) {
    if (offset < 0 || offset > size()) {
        throw new IndexOutOfBoundsException();
    }

    // Ensure there is enough capacity for the new element
    if (tail == capacity) {
        resize();
    }

    // Shift elements to the right to make space for the new element
    System.arraycopy(data, head + offset, data, head + offset + 1, tail - (head + offset));

    // Insert the new value
    data[head + offset] = value;

    // Update the tail pointer
    tail++;
}

    @Override
    public void add(int[] values, int offset, int length) {
        if (offset < 0 || length < 0 || offset + length > values.length) {
            throw new IndexOutOfBoundsException();
        }
        while (tail + length > capacity) {
            resize();
        }
        System.arraycopy(values, offset, data, tail, length);
        tail += length;
    }

@Override
public int getNoEntryValue() {
    return Integer.MIN_VALUE; // Custom no-entry value for TIntList
}

@Override
public void add(int[] values) {
    // Ensure there is enough capacity for the new elements
    while (tail + values.length > capacity) {
        resize();
    }

    // Copy values from the provided array into the list
    System.arraycopy(values, 0, data, tail, values.length);

    // Update the tail pointer
    tail += values.length;
}

    @Override
    public boolean removeAll(int[] values) {
        boolean modified = false;
        for (int value : values) {
            modified |= remove(value);
        }
        return modified;
    }

    @Override
    public boolean removeAll(TIntCollection collection) {
        boolean modified = false;
        TIntIterator iterator = collection.iterator();
        while (iterator.hasNext()) {
            int value = iterator.next();
            modified |= remove(value);
        }
        return modified;
    }

    @Override
    public boolean retainAll(int[] values) {
        boolean modified = false;
        for (int i = head; i < tail; ) {
            if (!containsValue(data[i], values)) {
                removeAt(i - head);
                modified = true;
            } else {
                i++;
            }
        }
        return modified;
    }

    private boolean containsValue(int value, int[] values) {
        for (int v : values) {
            if (v == value) {
                return true;
            }
        }
        return false;
    }

@Override
public boolean retainAll(TIntCollection collection) {
    boolean modified = false;
    for (int i = head; i < tail; ) {
        if (!collection.contains(data[i])) {
            removeAt(i - head);
            modified = true;
        } else {
            i++;
        }
    }
    return modified;
}

@Override
public boolean addAll(int[] values) {
    // Ensure there is enough capacity for the new elements
    while (tail + values.length > capacity) {
        resize();
    }

    // Copy values from the provided array into the list
    System.arraycopy(values, 0, data, tail, values.length);

    // Update the tail pointer
    tail += values.length;

    // Return true if the list was modified
    return values.length > 0;
}

@Override
public boolean containsAll(int[] values) {
    for (int value : values) {
        if (!contains(value)) {
            return false;
        }
    }
    return true;
}

@Override
public boolean addAll(TIntCollection collection) {
    // Ensure there is enough capacity for the new elements
    while (tail + collection.size() > capacity) {
        resize();
    }

    // Copy values from the provided collection into the list
    TIntIterator iterator = collection.iterator();
    while (iterator.hasNext()) {
        data[tail++] = iterator.next();
    }

    // Return true if the list was modified
    return !collection.isEmpty();
}

@Override
public boolean containsAll(TIntCollection collection) {
    TIntIterator iter = collection.iterator();
    while (iter.hasNext()) {
        if (!contains(iter.next())) {
            return false;
        }
    }
    return true;
}

@Override
public boolean containsAll(Collection<?> collection) {
    for (Object obj : collection) {
        if (!(obj instanceof Integer) || !contains((Integer) obj)) {
            return false;
        }
    }
    return true;
}

    @Override
    public TIntList subList(int fromIndex, int toIndex) {
        if (fromIndex < 0 || toIndex > size() || fromIndex > toIndex) {
            throw new IndexOutOfBoundsException();
        }
        ShiftToMiddleArray sublist = new ShiftToMiddleArray(toIndex - fromIndex);
        System.arraycopy(data, head + fromIndex, sublist.data, sublist.head, toIndex - fromIndex);
        sublist.tail = sublist.head + (toIndex - fromIndex);
        return sublist;
    }	

    }
}
