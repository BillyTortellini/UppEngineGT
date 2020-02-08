#ifndef __ALLOCATORS_HPP__
#define __ALLOCATORS_HPP__

#include "..\datatypes.hpp"
#include <cstddef>

typedef void* (*allocFunc)(void* allocator, size_t size);
typedef void (*freeFunc)(void* allcator, void* p);

struct Allocator
{
    allocFunc alloc;
    freeFunc free;
    void* allocator;
};

void* systemAlloc(void* unused, size_t size) {
    return malloc(size);
}

void systemFree(void* unused, void* p) {
    free(p);
}

Allocator systemAllocator;
void initAllocators() {
    systemAllocator.alloc = &systemAlloc;
    systemAllocator.free = &systemFree;
    systemAllocator.allocator = nullptr;
}

Allocator getSystemAllocator() {
    return systemAllocator;
}

void* alloc(Allocator* allocator, size_t size) {
    return allocator->alloc(allocator->allocator, size);
}

void free(Allocator* allocator, void* p) {
    allocator->free(allocator->allocator, p);
}

struct StackAllocator
{
    byte* memory;
    size_t capacity;
    size_t used;
    Allocator allocator;
    bool freeOnShutdown;
};

void init(StackAllocator* s, void* memory, size_t capacity)
{
    s->memory = (byte*) memory;
    s->capacity = capacity;
    s->used = 0;
    s->freeOnShutdown = false;
}

void init(StackAllocator* s, Allocator allocator, size_t capacity)
{
    s->capacity = capacity;
    s->used = 0;
    s->freeOnShutdown = true;
    s->allocator = allocator;
    s->memory = (byte*) alloc(&allocator, capacity);
}

void init(StackAllocator* s, size_t capacity) {
    init(s, getSystemAllocator(), capacity);
}

void shutdown(StackAllocator* s) {
    if (s->freeOnShutdown) {
        free(&s->allocator, s->memory);
    }
}

void* alloc(StackAllocator* s, size_t size) 
{
    // Round up to next multiple
    u64 start = (u64) (s->memory);
    u64 current = start + s->used;
    u64 rounded = current + (current % alignof(std::max_align_t));

    if (rounded + size >= start + s->capacity) {
        //TODO do something
        debugPrintf("Stack allocator ran out of memory!\n");
        debugWaitForConsoleInput();
        return nullptr;
    }

    s->used = (rounded - start + size);
    return (void*) rounded;
}

void free(StackAllocator* allocator, void* p) {
    return;
}

u64 createRollback(StackAllocator* s) {
    return s->used;
}

void rollback(StackAllocator* s, u64 rollbackToken) {
    s->used = rollbackToken;
}

void reset(StackAllocator* s) {
    s->used = 0;
}

Allocator getAllocator(StackAllocator* s)
{
    Allocator a;
    a.alloc = (allocFunc)(static_cast<void* (*)(StackAllocator*, size_t)>(&alloc));
    a.free = (freeFunc)(static_cast<void (*)(StackAllocator*, void*)>(&free));
    a.allocator = (void*) s;
    return a;
}

// ------ HEAP ALLOCATOR -------
struct SingleAlloc
{
    byte* memory;
    u64 size;
    SingleAlloc* next;
};

struct HeapAllocator
{
    byte* memory;
    u64 capacity;
    u64 highestAllocEnd;
    u32 allocationCount;
    SingleAlloc* head;
};

void* roundUp(void* addr)
{
    u64 intAddr = (u64)addr;
    return (void*) (intAddr + (intAddr % alignof(std::max_align_t)));
}

void* roundDown(void* addr)
{
    u64 intAddr = (u64)addr;
    u64 mod = alignof(std::max_align_t);
    return (void*) (intAddr - (mod - (intAddr % mod)));
}

void init(HeapAllocator* h, void* data, int capacity)
{
    h->memory = (byte*) data;
    h->capacity = capacity;
    h->highestAllocEnd = 0;
    h->allocationCount = 0;
    h->head = nullptr;
}

SingleAlloc* getNewSingleAlloc(HeapAllocator* h)
{
    SingleAlloc* newAlloc = nullptr;
    if (h->head == nullptr) 
    {
        // Check if enough memory is available
        newAlloc = (SingleAlloc*) roundDown(h->memory + h->capacity - sizeof(HeapAllocator));
        if ((byte*) h->head < h->memory + h->highestAllocEnd) {
            debugPrintf("Heap allocator ran out of memory, not enough space for Alloc entry");
            debugWaitForConsoleInput();
            return nullptr;
        }

        h->head = newAlloc;
    }
    else {
        // TODO Handle this code path
    }

    return newAlloc;
}

void* alloc(HeapAllocator* h, size_t size)
{
    // Create new Allocation entry
    SingleAlloc* newAlloc;
return nullptr;
}









#endif
