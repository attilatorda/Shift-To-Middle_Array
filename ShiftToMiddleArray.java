import java.util.*;
import java.util.concurrent.ThreadLocalRandom;

public class ShiftToMiddleArrayBenchmark {

    public static void main(String[] args) {
        runBenchmark(10000);
    }

    public static void runBenchmark(int operations) {
        int[] testSizes = {0, 10, 100, 1000, 5000, 10000};

        for (int size : testSizes) {
            System.out.println("Test size: " + size);

            System.out.println("Benchmarking ArrayList...");
            double arrayListTime = benchmarkRandomOperations(new ArrayList<>(), size, operations);
            System.out.println("ArrayList: " + arrayListTime + " ms");

            System.out.println("Benchmarking LinkedList...");
            double linkedListTime = benchmarkRandomOperations(new LinkedList<>(), size, operations);
            System.out.println("LinkedList: " + linkedListTime + " ms");

            System.out.println("Benchmarking ShiftToMiddleArray...");
            double stmTime = benchmarkRandomOperations(new ShiftToMiddleArray<>(), size, operations);
            System.out.println("ShiftToMiddleArray: " + stmTime + " ms\n");
        }
    }

public static <E> double benchmarkRandomOperations(List<E> list, int size, int operations) {
    ThreadLocalRandom rng = ThreadLocalRandom.current();
    int[] opDist = {30, 30, 30, 10}; // 10% chance for spike
    boolean spikeMode = false;
    int storedValue = 0; // Prevent compiler optimizations

    long start = System.nanoTime();

    for (int i = 0; i < 10; ++i) { // 10 iterations
        // Initial insertions
        for (int j = 0; j < size; ++j) {
            list.add((E) Integer.valueOf(j));
        }

        // Mixed random operations
        for (int j = 0; j < operations; ++j) {
            if (list.isEmpty()) continue;
            int index = rng.nextInt(list.size());

            int op = rng.nextInt(100);
            if (op < opDist[0]) { // Insert at random position
                if (index < list.size()) list.set(index, (E) Integer.valueOf(j));
                else list.add((E) Integer.valueOf(j));
            } else if (op < opDist[0] + opDist[1]) { // Remove if not empty
                if (index < list.size()) list.remove(index);
            } else if (op < opDist[0] + opDist[1] + opDist[2]) { // Read element
                if (index < list.size()) {
                    E element = list.get(index);
                    if (element != null) {
                        storedValue = (Integer) element;
                    }
                }
            } else { // Spike event: randomly remove/add 10% of elements
                int spikeSize = list.size() / 10;
                for (int k = 0; k < spikeSize; ++k) {
                    int spikeIndex = rng.nextInt(list.size());

                    if (spikeMode && !list.isEmpty()) {
                        list.remove(spikeIndex);
                    } else {
                        list.add(spikeIndex, (E) Integer.valueOf(k));
                    }
                }
                spikeMode = !spikeMode; // Alternate spike behavior
            }
        }
    }

    long end = System.nanoTime();
    return (end - start) / 1_000_000.0; // Convert to milliseconds
}

static class ShiftToMiddleArray<E> implements List<E> {
    private Object[] data;
    private int head, tail, capacity;

    public ShiftToMiddleArray(int initialCapacity) {
        this.capacity = initialCapacity;
        this.data = new Object[capacity];
        this.head = capacity / 2;
        this.tail = head;
    }

    public ShiftToMiddleArray() {
        this(16);
    }

    private void resize() {
        int size = tail - head;

        if (size < capacity - 2) {
            shift(size);
            return;
        }

        int newCapacity = capacity * 2;
        Object[] newData = new Object[newCapacity];
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
    public boolean contains(Object o) {
        for (int i = head; i < tail; i++) {
            if (Objects.equals(data[i], o)) return true;
        }
        return false;
    }

    @Override
    public Iterator<E> iterator() {
        return new Iterator<E>() {
            private int current = head;

            @Override
            public boolean hasNext() {
                return current < tail;
            }

            @Override
            public E next() {
                if (!hasNext()) throw new NoSuchElementException();
                return (E) data[current++];
            }
        };
    }

    @Override
    public Object[] toArray() {
        Object[] array = new Object[size()];
        System.arraycopy(data, head, array, 0, size());
        return array;
    }

    @Override
    public <T> T[] toArray(T[] a) {
        if (a.length < size()) {
            a = Arrays.copyOf(a, size());
        }
        System.arraycopy(data, head, a, 0, size());
        return a;
    }

    @Override
    public boolean add(E e) {
        if (tail == capacity) resize();
        data[tail++] = e;
        return true;
    }

    @Override
    public boolean remove(Object o) {
        for (int i = head; i < tail; i++) {
            if (Objects.equals(data[i], o)) {
                remove(i - head);
                return true;
            }
        }
        return false;
    }

    @Override
    public boolean containsAll(Collection<?> c) {
        for (Object o : c) {
            if (!contains(o)) return false;
        }
        return true;
    }

    @Override
    public boolean addAll(Collection<? extends E> c) {
        for (E e : c) {
            add(e);
        }
        return true;
    }

    @Override
    public boolean addAll(int index, Collection<? extends E> c) {
        if (index < 0 || index > size()) throw new IndexOutOfBoundsException();
        for (E e : c) {
            add(index++, e);
        }
        return true;
    }

    @Override
    public boolean removeAll(Collection<?> c) {
        boolean modified = false;
        for (Object o : c) {
            modified |= remove(o);
        }
        return modified;
    }

    @Override
    public boolean retainAll(Collection<?> c) {
        boolean modified = false;
        for (int i = head; i < tail; i++) {
            if (!c.contains(data[i])) {
                remove(i - head);
                modified = true;
            }
        }
        return modified;
    }

    @Override
    public void clear() {
        head = tail = capacity / 2;
    }

    @Override
    public E get(int index) {
        if (index < 0 || index >= size()) throw new IndexOutOfBoundsException();
        return (E) data[head + index];
    }

    @Override
    public E set(int index, E element) {
        if (index < 0 || index >= size()) throw new IndexOutOfBoundsException();
        E oldValue = (E) data[head + index];
        data[head + index] = element;
        return oldValue;
    }

    @Override
    public void add(int index, E element) {
        if (index < 0 || index > size()) throw new IndexOutOfBoundsException();

        int mid = (head + tail) / 2;
        if (index < mid && head > 0) {
            // Shift head left
            head--;
            for (int i = head; i < index; i++) {
                data[i] = data[i + 1];
            }
            data[index] = element;
        } else if (tail < capacity) {
            // Shift tail right
            for (int i = tail; i > index; i--) {
                data[i] = data[i - 1];
            }
            data[index] = element;
            tail++;
        } else {
            // Resize if needed
            resize();
            add(index, element);
        }
    }

    @Override
    public E remove(int index) {
        if (index < 0 || index >= size()) throw new IndexOutOfBoundsException();
        E oldValue = (E) data[head + index];
        for (int i = head + index; i < tail - 1; i++) {
            data[i] = data[i + 1];
        }
        tail--;
        return oldValue;
    }

    @Override
    public int indexOf(Object o) {
        for (int i = head; i < tail; i++) {
            if (Objects.equals(data[i], o)) return i - head;
        }
        return -1;
    }

    @Override
    public int lastIndexOf(Object o) {
        for (int i = tail - 1; i >= head; i--) {
            if (Objects.equals(data[i], o)) return i - head;
        }
        return -1;
    }

    @Override
    public ListIterator<E> listIterator() {
        return listIterator(0);
    }

    @Override
    public ListIterator<E> listIterator(int index) {
        return new ListIterator<E>() {
            private int current = head + index;

            @Override
            public boolean hasNext() {
                return current < tail;
            }

            @Override
            public E next() {
                if (!hasNext()) throw new NoSuchElementException();
                return (E) data[current++];
            }

            @Override
            public boolean hasPrevious() {
                return current > head;
            }

            @Override
            public E previous() {
                if (!hasPrevious()) throw new NoSuchElementException();
                return (E) data[--current];
            }

            @Override
            public int nextIndex() {
                return current - head;
            }

            @Override
            public int previousIndex() {
                return current - head - 1;
            }

            @Override
            public void remove() {
                throw new UnsupportedOperationException();
            }

            @Override
            public void set(E e) {
                ShiftToMiddleArray.this.set(current - 1, e);
            }

            @Override
            public void add(E e) {
                throw new UnsupportedOperationException();
            }
        };
    }

    @Override
    public List<E> subList(int fromIndex, int toIndex) {
        throw new UnsupportedOperationException();
    }
}
}
