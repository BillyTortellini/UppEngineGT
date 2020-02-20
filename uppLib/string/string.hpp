#ifndef __STRING_HPP__
#define __STRING_HPP__

// ----------------------------
// ----- WORK IN PROGRESS -----
// ----------------------------
// I currently do not see how strings are really usefull.
// They probably should make it easier to use Allocators and string.

// Answer:
// Strings would be nice because allocators return blocks instead of
// pointers. That means working with strings is pretty shitty without allocators
class String
{
public:
    Blk str;
    Allocator* alloc;
    int length;

    void init(Allocator* alloc, const char* str) 
    {
        this->alloc = alloc;
        int len = (int)strlen(str);
        this->str = alloc->alloc(len+1); 
        strcpy((char*)this->str.data, str);
        this->length = len;
    }

    void cat(const char* s)
    {
        int len = (int)strlen(s);
        Blk b = alloc->alloc(len + length + 1);
        strcpy((char*)b.data, (char*)str.data);
        strcat((char*)b.data, s);
        alloc->dealloc(str);
        str = b;
        length = len + length;
    }

    char* c_str() {
        return (char*)str;
    }

    void shutdown() {
        alloc->dealloc(str);
    }

    int size() {
        return length;
    }
};


// Requirements:
// -------------
// Strings need to be usable with allocators.
// It should be easy to work with strings.
//
// Easy:
// str("Something") + " " + str(15) would be cool
// str("Something") would be nice
// str(15.17f);
//
// We want string because working with char* is tedious
// char** filenames = malloc(sizeof(char*) * 5);
// size_t len = strlen(asdf);
// filenames[2] = malloc(sizeof(char) * len);
// strcpy(filenames[2], asdf);
//
// String* arr = malloc(sizeof(String) * 5);
// test[0].init(asdf); // Saves 2 lines
//
// Maybe String.format(
// setStringAllocator
// pushStringAllocator
// popStringAllocator
//
// This should work
// String s = "asdf";
// printf("%s", &s);








#endif
