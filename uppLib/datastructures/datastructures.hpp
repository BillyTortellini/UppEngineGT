#ifndef __DATASTRUCTURES_HPP__
#define __DATASTRUCTURES_HPP__

// This file contains datastructures compatible with UppLib Allocators.
// Following datastructures are available:

// Data Containers:
// ---------------
//  - Array (Allocates on a allocator)
//  - List (Doubly linked)
//  - DynArray

// Searchables:
// ------------
//  - Hashmap

// Fixed Size Containers:
// ----------------------
// These containers use templates and allocate
// data on the stack, and do not require shutdown
// because they will be freed when they are out of scope
//  - FixedArray 
//  - FixedList
//  - FixedHashmap


// ------------------
// --- FIXED SIZE ---
// ------------------

// Todo: Remember how to do iterators
//      and lambdas in c++

// -------------------
// -- Dynamic Sized --
// -------------------
#define DYNARR_GROWTH_FACTOR 2
template <typename T>
class DynArr
{
public:
    Allocator* alloc;
    Blk data;
    int count;
    int capacity;

    DynArr(){};

    void init(Allocator* alloc, int capacity=8) {
        this->alloc = alloc;
        this->capacity = capacity;
        this->count = 0;
        if (capacity != 0) 
            data = alloc->alloc(capacity * sizeof(T));
    }

    void shutdown() {
        if (capacity != 0) {
            alloc->dealloc(data);
        }
        count = 0;
        capacity = 0;
    }

    void realloc(int capacity) 
    {
        bool shouldDealloc = this->capacity != 0;
        this->capacity = capacity;
        // Create new data and copy
        Blk newData = alloc->alloc(capacity * sizeof(T));
        memcpy(newData.data, data.data, count * sizeof(T));

        // Dealloc and set new data
        if (shouldDealloc) {
            alloc->dealloc(data);
        }
        data = newData;
    }

    // Makes sure that capacity is at least this big
    void reserve(int capacity) {
        if (this->capacity < capacity) {
            realloc(capacity);
        }
    }

    T& operator[](int index) 
    {
        // Check if resize is necessary
        if (index >= capacity) {
            int newCapacity = capacity * DYNARR_GROWTH_FACTOR;
            if (capacity == 0) {
                newCapacity = index+1;
            }
            else {
                while (newCapacity <= index) {
                    newCapacity *= DYNARR_GROWTH_FACTOR;
                }
            }
            realloc(newCapacity); 
        }
        if (count <= index) {
            count = index+1;
        }

        return ((T*)data.data)[index];
    }

    void push_back(const T& a) {
        (*(this))[count] = a;
    }

    void swap_remove(int index) {
        assert(index < count && index >= 0, "swap remove called with invalid index\n");
        T* arr = (T*)data.data;
        arr[index] = arr[count-1]; 
        count--;
    }

    void reset() {
        count = 0;
    };

    int size() {
        return count;
    }

    void swap(int i1, int i2) {
        if(i1==i2) return;
        T* arr = (T*)data.data;
        T tmp = arr[i1];
        arr[i1] = arr[i2];
        arr[i2] = tmp;
    }

    void sort(const std::function<int(T* a, T*b)>& comparator)
    {
        if (count == 0 || count == 1) return;

        T* arr = (T*)data.data;
        for (int i = 0; i < count; i++)
        {
            int minIndex = i;
            for (int j = i+1; j <count; j++) 
            {
                if (comparator(arr+j, arr+minIndex) == -1) 
                {
                    minIndex = j;
                }
            }
            swap(i, minIndex);
        }
    }

    struct Iterator
    {
        Iterator(int index, DynArr* arr)
            : index(index), arr(arr) {}

        bool operator==(const Iterator& o) {
            return o.index == index;
        }
        bool operator!=(const Iterator& o) {
            return o.index != index;
        }
        T& operator*() {
            return (*arr)[index];
        }
        T& operator->() {
            return (*arr)[index];
        }
        Iterator& operator++() {
            ++index;
            return *this;
        }
        Iterator operator++(int i) {
            Iterator it(index, arr);
            index++;
            return it;
        }

        int index; 
        DynArr* arr;
    };

    Iterator begin() {
        return Iterator(0, this);
    }

    Iterator end() {
        return Iterator(count, this);
    }
};
#endif
