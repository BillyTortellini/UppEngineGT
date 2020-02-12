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
        test_alloc(&stack);
        loggf("Stack p: %ld\n", stack.p);
        stack.reset();
        loggf("Stack p: %ld\n", stack.p);
        stack.shutdown();
    }
    logg("\n");

    // Test block allocator
    {
        logg("Block Allocator:\n");
        BlockAllocator ba;
        ba.init(&sa, 1024*1024, 3);

        test_alloc(&ba);

        ba.shutdown();
    }
    logg("\n");

    // Test List allocator
    {
        logg("List Allocator:\n");
        ListAllocator la;
        la.init(&sa, 1024*1024*3);

        test_alloc(&la);

        logg("\nCustom List test\n");
        la.shutdown();
        la.init(&sa, 1024);

        Blk b1 = la.alloc(256);
        la.print();
        Blk b2 = la.alloc(200);
        la.print();
        Blk b3 = la.alloc(256);
        Blk b4 = la.alloc(256);
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

        // Should fail
        loggf("\nDeallos that should fail\n");
        //la.dealloc(b4);
        //la.dealloc(b2);

        la.shutdown();
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




