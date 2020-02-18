#ifndef __ALLOCATORS_HPP__
#define __ALLOCATORS_HPP__

// ------------------
// --- ALLOCATORS ---
// ------------------
// Following allocators are implemented in this file:
//
//  Test allocators:
//  - NullAllocator (For testing only)
//  - PrintAllocator (Used for debugging, prints all alloc and malloc calls) 
//  - SystemAllocator (Uses malloc and free)
//
//  Fixed Size allocators:
//  - StackAllocator (Uses a stack, only limited dealloc functionality)
//  - BlockAllocator (Allocates fixed sized blocks, alloc and dealloc in O(1))
//  - ListAllocator (Saves all allocations in a linked list, alloc and dealloc in O(n))
//
//  Composition Allocators:
//  - FallbackAllocator (Calls another allocator if one fails)
//  - SegregateAllocator (Depending on threshhold, calls different allocators)

//  Not yet implemented:
//  - Composers:
//      * Bucketizer (Linear buckets)
//      * CascadingAllocator (List of allocators, grow lazily)
//  - Fixed size:
//      * BuddySystem (Power of 2 allocator)
//      * BitmappedBlock (Block allocator using bitmap)
//      * FreeList (Sits on top of another allocator, keeps sizes)


#include "../utils/datatypes.hpp"
#include "../utils/debugging_tools.hpp"
#include "../math/umath.hpp"
#include <stddef.h> // Defines max_align_t

// This is THE memory management unit, alloc and dealloc
// both use this structure.
struct Blk
{
    Blk(){}
    Blk(void* data, u64 size) : data(data), size(size){}
    void* data;    
    u64 size;

    // Allow conversion from Blk to any pointer type
    template<typename T>
    explicit operator T*() {return (T*) data;}
};

bool equals(const Blk& a, const Blk& o) {
    return a.data == o.data && a.size == o.size;
}

void* blkEnd(const Blk& b) {
    return (void*) ((u64)b.data + b.size);
}

void* blkEnd(Blk* b) {
    return (void*) ((u64)b->data + b->size);
}

Interval<u64> toInterval(const Blk& b) {
    return Interval<u64>((u64)b.data, (u64)b.data + b.size);
}

// All allocators return aligned data
void* roundToAligned(void* p) {
 return (void*) ceil((u64)p, alignof(max_align_t));
}

u64 roundToAligned(u64 p) {
    return ceil(p, alignof(max_align_t));
}

// Interface for all allocators.
class Allocator
{
public:
    virtual Blk alloc(u64 size) = 0;
    virtual void dealloc(const Blk& b) = 0;

    // Some allocators may implement:
    //  - bool owns(const Blk& b);
    //  - void reset();
    //  - void init(Blk b);
    //  - void init(Allocator* a, ...)
    //  - shutdown()    (Used after init)
    //  - allocAll() // Allocats all available space
    //  - count() // Returns allocation count if available
};

// DEBUG ALLOCATORS:
class NullAllocator : public Allocator
{
public:
    Blk alloc(u64 size) {
        return Blk(nullptr, 0);
    }

    void dealloc(const Blk& b) {
        return;
    }

    bool owns(Blk b) {
        return b.data == nullptr;
    }
};

template <typename T>
class PrintAllocator : public Allocator
{
public:
    PrintAllocator() {}
    PrintAllocator(T* alloc, const char* allocText, const char* deallocText) {
        init(alloc, allocText, deallocText);
    }
    PrintAllocator(T* alloc) {
        init(alloc);
    }
    void init(T* allocator, const char* allocText, const char* deallocText) {
        new(this) PrintAllocator;
        this->allocator = allocator;
        this->allocText = allocText;
        this->deallocText = deallocText;
    }

    void init(T* allocator) {
        init(allocator, "Allocated: ", "Deallocated: ");
    }

    Blk alloc(u64 size) {
        Blk b = allocator->alloc(size); 
        loggf("%srequest %ld\t data %p\tsize %ld\n", allocText, size, b.data, b.size);
        return b; 
    }

    void dealloc(const Blk& b) {
        loggf("%sdata: %p\tsize: %ld\n", deallocText, b.data, b.size);
        allocator->dealloc(b);
    }

    bool owns(Blk b) {
        return allocator->owns(b);
    }

    T* allocator;
    const char* allocText;
    const char* deallocText;
};

class SystemAllocator : public Allocator
{
public: 
    void init() {
        new(this) SystemAllocator;
    }

    Blk alloc(u64 size) {
        return Blk(malloc(size), size);    
    };

    void dealloc(const Blk& b) {
        free(b.data);    
    };
};

// COMPOSER ALLOCATORS
template <typename P, typename F>
class FallbackAllocator : public Allocator
{
public: 
    FallbackAllocator() {};
    FallbackAllocator(P* primary, F* fallback) {
        init(primary, fallback);
    }
    void init(P* primary, F* fallback) {
        this->primary = primary;
        this->fallback = fallback;
        new(this) FallbackAllocator;
    }

    Blk alloc(u64 size) 
    {
        Blk b = primary->alloc(size);
        if (b.data == nullptr) {
            return fallback->alloc(size);
        }
        return b;
    }

    void dealloc(const Blk& b) {
        if (primary->owns(b)) {
            primary->dealloc(b);
        }
        else {
            fallback->dealloc(b);
        }
    };

    bool owns(const Blk& b) {
        return primary->owns(b) || fallback->owns(b);
    }

    P* primary;
    F* fallback;
};

template <typename L , typename U>
class SegregateAllocator : public Allocator
{
public:
    SegregateAllocator(){};
    SegregateAllocator(int threshold, L* underAlloc, U* overAlloc) {
        init(threshold, underAlloc, overAlloc);
    }

    void init(int threshold, L* underAlloc, U* overAlloc) 
    {
        this->threshold = threshold;
        this->underAlloc = underAlloc;
        this->overAlloc = overAlloc;
        new(this) SegregateAllocator;
    }

    Blk alloc(u64 size) {
        if (size <= threshold) {
            return underAlloc->alloc(size);
        }
        else {
            return overAlloc->alloc(size);
        }
    }

    void dealloc(const Blk& b) {
        if (b.size <= threshold) {
            underAlloc->dealloc(b);
        }
        else {
            overAlloc->dealloc(b);
        }
    }

    bool owns(const Blk& b) {
        if (b.size <= threshold) {
            return underAlloc->owns(b);
        }
        else {
            return overAlloc->owns(b);
        }
    }

    int threshold;
    L* underAlloc;
    U* overAlloc;
};

class StackAllocator : public Allocator
{
public:
    void init(Blk b) {
        assert((u64)b.data % alignof(max_align_t) == 0, "Init stack allocator with non aligned data");
        memAllocator = nullptr;
        stack = b;
        p = 0;
        new(this) StackAllocator;
    }

    void init(Allocator* a, u64 size)
    {
        assert(a != nullptr, "StackAlloc init was called with null");
        memAllocator = a;
        stack = a->alloc(size);
        p = 0;
        new(this) StackAllocator;
    }

    void shutdown() 
    {
        if (memAllocator) {
            memAllocator->dealloc(stack);
        }
        stack.data = nullptr;
        stack.size = 0;
    }

    Blk alloc(u64 size) 
    {
        // P is used because we assume that stack is aligned
        u64 nextP = ceil(p, alignof(max_align_t)); 
        if (nextP + size > stack.size) {
            return Blk(nullptr, 0);
        }

        Blk res((void*)((u64)stack.data + nextP), size);
        p = nextP + size;
        return res;
    }

    Blk allocAll() 
    {
        // P is used because we assume that stack is aligned
        u64 nextP = ceil(p, alignof(max_align_t)); 
        assert(nextP < stack.size, "Alloc all called with (almost) full stack\n");
        u64 size = stack.size - nextP;
        return alloc(size);
    }

    bool owns(const Blk& b) {
        return inside(Interval<u64>((u64)stack.data, (u64)stack.data + p), toInterval(b));
    }

    void dealloc(const Blk& b) 
    {
        if (((u64)b.data + b.size) == ((u64)stack.data + p)) {
            p = (u64)b.data - (u64)stack.data;        
        }
    }

    u64 createCheckpoint() {
        return p;
    }

    void rollback(u64 checkpoint) {
        assert(checkpoint < stack.size, "rollback called with invalid value");
        p = checkpoint;
    }

    void reset() {
        p = 0;
    }

    Allocator* memAllocator;
    Blk stack;
    u64 p;
};

class BlockAllocator : public Allocator
{
public:
    void* head;
    u64 blockSize;
    u64 blockCount;
    Allocator* a;
    Blk memory;

    void* getBlockByIndex(u64 i) {
        return (void*)((u64)memory.data + i * blockSize);
    }

    void setBlockPtr(void* block, void* pointTo) {
        *((void**)block) = pointTo; 
    }

    void setBlockPtr(u64 i, void* pointTo) {
        setBlockPtr(getBlockByIndex(i), pointTo);   
    }

    void* getNextBlock(void* block) {
        return *((void**)block);
    }

    void setupBlocks()
    {
        // Set last block to point to zero
        setBlockPtr(blockCount-1, nullptr);
        // Set all blocks to point to the next one
        for (int i = 0; i < blockCount-1; i++)
        {
            void* block = getBlockByIndex(i);
            void* next = getBlockByIndex(i+1);
            setBlockPtr(block, next);
        }
        head = getBlockByIndex(0);
    }

    void init(const Blk& b, u64 blockSize)
    {
        this->a = nullptr;
        memory = b;
        this->blockSize = max(blockSize, sizeof(void*));
        this->blockCount = b.size / this->blockSize;
        setupBlocks();
        new(this) BlockAllocator;
    }

    void init(Allocator* a, u64 blockSize, u64 blockCount)
    {
        this->a = a;
        this->blockSize = max(blockSize, sizeof(void*));
        this->blockCount = blockCount;
        memory = a->alloc(blockCount*this->blockSize);
        setupBlocks();
        new(this) BlockAllocator;
    }

    void shutdown() 
    {
        if (a != nullptr) {
            a->dealloc(memory);
        }
        memory.size = 0;
        memory.data = 0;
    }

    Blk alloc(u64 size) {
        if (size > blockSize) {
            return Blk(nullptr, 0);
        }
        Blk result;
        result.data = head;
        result.size = blockSize;
        if (head != nullptr) {
            head = getNextBlock(head);
        }
        return result;
    }

    void dealloc(const Blk& b) {
        assert(b.size <= blockSize, "Block allocator dealloc bigger than block\n");
        assert(owns(b), "Block deallocated does not belong to allocator\n");

        void* oldHead = head;
        head = b.data;
        setBlockPtr(head, oldHead);
    }

    bool owns(const Blk& b){
        return inside(toInterval(b), toInterval(memory));
    }

    // Returns the number of allocations
    int count() 
    {
        // Count number of free blocks
        int freeCount = 0;
        void* it = head;
        while (it != nullptr) {
            it = getNextBlock(it);
            freeCount++;
        }
        return (int) (blockCount - freeCount);
    }
};

struct ListAllocNode
{
    ListAllocNode* next;
    Blk memory;
};

// Used in list allocator printing
int readablePtr(void* p) {
    return (int)((u64)p % (2<<16));
}

class ListAllocator : public Allocator
{
public:
    Allocator* parent;
    Blk memory;

    void resetHead() {
        ListAllocNode* head = getHead();
        head->next = nullptr;
        head->memory.data = roundToAligned((void*)((u64)head + sizeof(ListAllocNode)));
        head->memory.size = 0;
    }

    ListAllocNode* getHead() {
        return (ListAllocNode*) memory.data;
    }

    void init(Allocator* parent, u64 size) {
        this->parent = parent;
        memory = parent->alloc(size);
        resetHead();
        new(this) ListAllocator;
    }

    void init(const Blk& b) {
        parent = nullptr;
        memory = b;
        resetHead();
        new(this) ListAllocator;
    }

    void shutdown() {
        if (parent != nullptr) {
            parent->dealloc(memory);
        }
        memory.data = nullptr;
        memory.size = 0;
    }

    bool fitsAfter(ListAllocNode* n, u64 size) 
    {
        u64 bEnd = (u64)blkEnd(n->memory);
        u64 nextNode = ceil(bEnd, alignof(ListAllocNode));
        u64 end = roundToAligned(nextNode + sizeof(ListAllocNode));
        end += size;

        u64 nextStart;
        // Check if this is the last node
        if (n->next == nullptr) {
            nextStart = (u64) blkEnd(memory);
        }
        else {
            nextStart = (u64) n->next;
        }

        return end <= nextStart; // Maybe >=, but for safety >
    }

    void addNode(ListAllocNode* n, ListAllocNode* after) 
    {
        n->next = after->next;
        after->next = n;
    }

    Blk alloc(u64 size) 
    {
        // Search for fitting area in nodeList 
        bool found = false;
        ListAllocNode* it = getHead();
        while (it != nullptr) {
            // Check if fits after
            if (fitsAfter(it, size)) {
                found = true;
                break;
            }
            it = it->next;
        }

        if (!found) {
            return Blk(nullptr, 0);
        }

        // Create new node and add it 
        ListAllocNode* n = (ListAllocNode*) ceil((u64)blkEnd(it->memory), alignof(ListAllocNode));

        n->memory.data = (void*) roundToAligned((u64) n + sizeof(ListAllocNode));
        n->memory.size = size;

        addNode(n, it);

        return n->memory;
    }

    void debugPrintNode(ListAllocNode* n) {
        loggf("\t Data: %d\t size: %ld\n", readablePtr(n->memory.data), n->memory.size);
    }

    void print() {
        logg("Printing list alloc:\n");
        ListAllocNode* it = getHead();
        loggf("\tMemory start: %d\n", readablePtr(memory.data));
        while (it != nullptr) {
            debugPrintNode(it);
            it = it->next;
        }
        loggf("\tMemory End: %d\n", readablePtr(blkEnd(memory)));
    }

    void dealloc(const Blk& b) 
    {
        assert(b.data != memory.data, "Error, should never happen since this is list");
        assert(!equals(b, getHead()->memory), "Something wrong happened");

        ListAllocNode* prev = getHead();
        ListAllocNode* curr = prev->next;
        while (curr != nullptr) 
        {
            if (equals(curr->memory, b))
            {
                // Found block, remove it
                prev->next = curr->next;        
                // This is for safety
                curr->next = nullptr;
                curr->memory.data = nullptr;
                curr->memory.size = 0;

                return;
            }
            prev = curr;
            curr = curr->next;
        }

        // Not found
        invalid_path("Dealloc of list did not find allocation");
    }

    int count() {
        ListAllocNode* n = getHead();
        int c = 0;
        while (n != nullptr) {
            c++;
            n = n->next;
        }
        return c - 1; // Because head always exists in current impl
    }

    bool existsInList(const Blk& b) 
    {
        ListAllocNode* n = getHead();
        while (n != nullptr) 
        {
            if (equals(n->memory, b)) {
                return true;
            }
            n = n->next;
        }

        return false;
    }

    bool owns(const Blk& b) {
        // Owns should be simplified to this for performance reasons
        //return inside(toInterval(b), toInterval(memory));
        
        // But for debugging savety, we will use this instead:
        return existsInList(b);
    }
};




#endif
