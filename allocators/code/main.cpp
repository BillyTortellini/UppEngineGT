#include <cstdio>
#include <cstdlib>

// TODO:
// Allocator types:
//  * Stack allocator
//  * Block allocator
//  * Heap allocator
//  * Default allocator

typedef unsigned char byte;


void string_test()
{
    String str;
    init(str, char*);
    string str = "";
}


int main(int argc, char** argv)
{
    // Allocators:
    // Why use allocators?
    // Because we do not use malloc/free (new and delete) in the game,
    // since all the memory is initialized at once at startup to a fixed,
    // predefined size (4GB). After that, we use this fixed, given memory
    // for the runtime of the game.

    // This is done because we want to be able to run with a fixed amount of memory given.
    // It also avoids round trips and operating system calls, which is nice.

    // But we have a problem if we want to use dynamic data structures. Sinces types
    // like dynamic arrays, lists, hash_maps and the sorts need some way of allocating new data, 
    // it would be nice if we could do that.
    
    // Idea 1: Object oriented approach
    // Make a base class called allocator, and let the different subtypes be different kinds of allocators.
    // Now datastructues could use the base allocator to allocate new memory.

    // Idea 2: C-Style approach mimicking OO
    // Only use a pointer that has 
    StackAlloc s = constructStack(;
    init(&s, data, 1024);

    DynamicArray a;


	return 0;
}
