#ifndef __STRING_HPP__
#define __STRING_HPP__

#include "..\datatypes.hpp"
#include <cstring>
#include "allocators.hpp"

struct String
{
    char* characters;
    int length;
    Allocator allocator;
};

void init(String* s, const char* str, Allocator allocator)
{
    s->length = (int) strlen(str);
    s->characters = (char*) alloc(&allocator, s->length + 1);
    memcpy(s->characters, str, s->length + 1);
}

void init(String* s, const char* str) {
    init(s, str, getSystemAllocator());
}

void shutdown(String* s)
{
    free(&s->allocator, s->characters);
}

bool endsWith(String* s, const char* end)
{
    int length = (int) strlen(end);
    if (length > s->length) {
        return false;
    }

    return strcmp(s->characters + s->length - length, end) == 0;
}

const char* c_str(String* s) {
    return s->characters;
}
















#endif
