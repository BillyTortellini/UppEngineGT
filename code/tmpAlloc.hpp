#ifndef __TMP_ALLOC__
#define __TMP_ALLOC__

#define TMP_ALLOC_SIZE 1024 * 1024 * 10
StackAllocator tmpAlloc;

#define CONCAT_ARGS_(x, y) x ## y
#define CONCAT_ARGS(x, y) CONCAT_ARGS_(x, y)
#define SCOPE_EXIT_ROLLBACK \
    u64 CONCAT_ARGS(_checkpoint_, __LINE__) = tmpAlloc.createCheckpoint(); \
    SCOPE_EXIT(tmpAlloc.rollback(CONCAT_ARGS(_checkpoint_, __LINE__));)

void initTmpAlloc(Allocator* alloc) {
    tmpAlloc.init(alloc, TMP_ALLOC_SIZE);
}

void shutdownTmpAlloc() {
    tmpAlloc.shutdown();
}



#endif
