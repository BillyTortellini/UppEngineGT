#include <cstdio>

#include "../uppLib.hpp"

// ------------------------------------------------
// --- This is a test file for uppLib functions ---
// ------------------------------------------------

void test_scopedExit()
{
    SCOPE_EXIT (
        printf("Exit 3\n");
    );
    {
        SCOPE_EXIT(printf("Exit 1\n"));
        printf("Exit 0\n");
    }
    printf("Exit 2\n");
}

#include <iostream>
void customDebugLogg(const char* str) {
    std::cout << "CUSTOM: " << str << std::endl;
}

void customInvalidPath(const char* msg) {
    std::cout << "CUSTOM INVALID PATH LOL" << msg << std::endl;
    std::cin.ignore();
}

void test_debug_tools() {
    logg("Hello there, general kenobi!\n");
    int x = 5 * 2;
    // logg, loggf, invalid_path, assert, setDebugFunctions
    logg("Default logg\n");
    setDebugFunctions(&customDebugLogg, &customInvalidPath);
    logg("My custom logg\n");
    invalid_path("custom path not valid\n");

    printf("SHOULD NOT REACH\n");
}

void test_alloc(Allocator* a)
{
    Blk b1 = a->alloc(128);
    Blk b2 = a->alloc(256);
    Blk b3 = a->alloc(1024*1024);
    loggf("b1 data: %p, size: %ld\n", b1.data, b1.size);
    loggf("b2 data: %p, size: %ld\n", b2.data, b2.size);
    loggf("b3 data: %p, size: %ld\n", b3.data, b3.size);
    a->dealloc(b3);
    a->dealloc(b1);
    a->dealloc(b2);
}

void test_allocators()
{
    // Test null allocator
    {
        logg("NullAllcoator:\n");
        NullAllocator na;
        test_alloc(&na);
    }
    logg("\n\n");

    // Test system allocator
    SystemAllocator sa;
    {
        logg("SystemAllocator:\n");
        test_alloc(&sa);
    }
    logg("\n");

    // Test stack allocator
    {
        logg("Stack Allocator:\n");
        StackAllocator stack;
        stack.init(&sa, 1024*1024);
        SCOPE_EXIT(stack.shutdown(););

        test_alloc(&stack);

        loggf("Stack p: %ld\n", stack.p);
        Blk all = stack.allocAll();
        loggf("Stack alloc all: size %ld, data: %p\n", all.size, all.data);
        stack.reset();
        loggf("Stack p: %ld\n", stack.p);
    }
    logg("\n");

    // Test block allocator
    {
        logg("BLOCK_ALLOCATOR START: \n");
        BlockAllocator ba;
        ba.init(&sa, 1024*1024, 3);

        test_alloc(&ba);
        test_alloc(&ba);
        test_alloc(&ba);
        loggf("Block count after test: %d\n", ba.count());

        Blk b1 = ba.alloc(23);
        loggf("Count should be 1: %d\n", ba.count());
        Blk b2 = ba.alloc(1025);
        loggf("Count should be 2: %d\n", ba.count());
        Blk b3 = ba.alloc(1024*1024);
        loggf("Count should be 3: %d\n", ba.count());
        
        ba.dealloc(b2);
        loggf("Count should be 2: %d\n", ba.count());
        ba.dealloc(b1);
        loggf("Count should be 1: %d\n", ba.count());
        ba.dealloc(b3);
        loggf("Count should be 0: %d\n", ba.count());

        ba.shutdown();
        logg("BLOCK_ALLOCATOR END \n");
    }
    logg("\n");

    // Test List allocator
    {
        logg("List Allocator:\n");
        ListAllocator la;
        la.init(&sa, 1024*1024*3);
        loggf("la.size() should be 0: %d\n", la.count());

        test_alloc(&la);

        logg("\nCustom List test\n");
        la.shutdown();
        la.init(&sa, 1024);

        Blk b1 = la.alloc(256);
        la.print();
        Blk b2 = la.alloc(200);
        la.print();
        loggf("la.count() should be 2: %d\n", la.count());
        Blk b3 = la.alloc(256);
        Blk b4 = la.alloc(256);
        loggf("la.count() should be 3: %d\n", la.count());
        loggf("b1 size: %ld, data: %p\n", b1.size, b1.data);
        loggf("b2 size: %ld, data: %p\n", b2.size, b2.data);
        loggf("b3 size: %ld, data: %p\n", b3.size, b3.data);
        loggf("b4 size: %ld, data: %p\n", b4.size, b4.data);
        la.print();
        la.dealloc(b2);
        la.print();

        loggf("\nDeallocs that should work\n");

        la.dealloc(b3);
        la.dealloc(b1);
        loggf("la.count() should be 0: %d\n", la.count());

        // Should fail
        loggf("\nDeallocs that should fail\n");
        //la.dealloc(b4);
        //la.dealloc(b2);

        la.shutdown();
    }

    // Test Fallback Allocator
    {
        loggf("\n\n TEST FALLBACK START\n");
        BlockAllocator primary;
        primary.init(&sa, 256, 10);
        SCOPE_EXIT(primary.shutdown());

        PrintAllocator printP(&primary, "Primary alloc: ", "Primary dealloc: ");
        PrintAllocator printF(&sa, "Fallback alloc: ", "Fallback dealloc: ");

        FallbackAllocator fallback(&printP, &printF);
        test_alloc(&fallback);
        loggf("\n\n TEST FALLBACK END\n");


        //FallbackAllocator<BlockAllocator
    }

    // Test more complex allocator
    {
        // TODO: StackAlloc allocAll();
        // Small allocators
        logg("\n\nSuper sick allocator: \n");
        BlockAllocator blockAlloc32, blockAlloc64, blockAlloc1024;
        blockAlloc32.init(&sa, 32, 1024); // 32 byte blocks
        blockAlloc64.init(&sa, 64, 256); // 64 byte blocks
        blockAlloc1024.init(&sa, 1024, 32); // 64 byte blocks
        SCOPE_EXIT(blockAlloc32.shutdown(); blockAlloc64.shutdown(); blockAlloc1024.shutdown());

        // Big allocator
        ListAllocator listAlloc;
        listAlloc.init(&sa, 1024L*1024L*64L); // 64 MB
        SCOPE_EXIT(listAlloc.shutdown());

        PrintAllocator printBlk32(&blockAlloc32, "B32 alloc: ", "B32 dealloc ");
        PrintAllocator printBlk64(&blockAlloc64, "B64 alloc: ", "B64 free: ");
        PrintAllocator printBlk1024(&blockAlloc1024, "B1024 alloc: ", "B1024 free: ");
        PrintAllocator printList(&listAlloc, "List alloc: ", "List free: ");

        SegregateAllocator segAlloc1024(1024, &printBlk1024, &printList);
        SegregateAllocator segAlloc64(64, &printBlk64, &segAlloc1024);
        SegregateAllocator segAlloc32(32, &printBlk32, &segAlloc64);

        // TEST:
        Allocator* alloc = &segAlloc32;
        Blk b1 = alloc->alloc(15);
        Blk b2 = alloc->alloc(32);
        Blk b3 = alloc->alloc(32);
        Blk b4 = alloc->alloc(33);
        Blk b5 = alloc->alloc(55);
        Blk b6 = alloc->alloc(64);

        Blk b7 = alloc->alloc(65);
        Blk b8 = alloc->alloc(512);
        Blk b9 = alloc->alloc(1024);
        Blk b10 = alloc->alloc(1024);
        Blk b11 = alloc->alloc(1024);
        Blk b12 = alloc->alloc(1025);

        loggf("Blk32 cnt should be 3: %d\n", blockAlloc32.count());
        loggf("Blk64 cnt should be 3: %d\n", blockAlloc64.count());
        loggf("Blk1024 cnt should be 5: %d\n", blockAlloc1024.count());
        loggf("List cnt should be 1: %d\n", listAlloc.count());

        alloc->dealloc(b4);
        alloc->dealloc(b5);
        alloc->dealloc(b6);
        alloc->dealloc(b1);
        alloc->dealloc(b2);
        alloc->dealloc(b3);

        alloc->dealloc(b7);
        alloc->dealloc(b8);
        alloc->dealloc(b9);
        alloc->dealloc(b10);
        alloc->dealloc(b11);
        alloc->dealloc(b12);

        loggf("Blk32 cnt should be 0: %d\n", blockAlloc32.count());
        loggf("Blk64 cnt should be 0: %d\n", blockAlloc64.count());
        loggf("Blk1024 cnt should be 0: %d\n", blockAlloc1024.count());
        loggf("List cnt should be 0: %d\n", listAlloc.count());
    }
    logg("\n");
}

int main(int argc, char** argv)
{
    //test_scopedExit();
    //test_debug_tools();
    test_allocators();

    return 0;
}




