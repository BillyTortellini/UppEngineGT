#ifndef __STRING_UTILS_HPP__
#define __STRING_UTILS_HPP__

#include <ctype.h> // for tolower
#include "tmpAlloc.hpp"

void toLower(char* buf)
{
    int len = (int) strlen(buf);
    for (int i = 0; i < len; i++) {
        buf[i] = (char)tolower(buf[i]);
    }
}

bool endsWith(const char* str, const char* end) {
    int endLen = (int) strlen(end);
    int strLen = (int) strlen(str);
    if (endLen > strLen) return false;
    return strcmp(end, &(str[strLen-endLen])) == 0;
}

bool startsWith(const char* str, const char* start) {
    int startLen = (int) strlen(start);
    int strLen = (int) strlen(str);
    if (startLen > strLen) return false;
    for (int i = 0; i < startLen; i++) {
        if (str[i] != start[i]) {
            return false;
        }
    }
    return true;
}

bool contains(const char* str, char c) {
    int len = (int) strlen(str);
    for (int i = 0; i < len; i++) {
        if (str[i] == c) {
            return true;
        }
    }

    return false;
}

// Temp string class
class TmpStr
{
public:
    char* str;
    int length;
    u64 checkpoint;

    TmpStr(const char* str) {
        length = (int)strlen(str);
        checkpoint = tmpAlloc.createCheckpoint();
        this->str = (char*) tmpAlloc.alloc(length+1);
        memcpy(this->str, str, length+1);
    }

    TmpStr(int length) {
        assert(length > 0, "TmpStr called with length <= 0\n");
        this->length = length;
        checkpoint = tmpAlloc.createCheckpoint();
        this->str = (char*) tmpAlloc.alloc(length+1);
        this->str[0] = '\0';
    }

    ~TmpStr() {
        tmpAlloc.rollback(checkpoint);
    }

    TmpStr operator+(const char* str) {
        SCOPE_EXIT_ROLLBACK;
        int len = (int)strlen(str);
        char* cat = (char*) tmpAlloc.alloc(len + length + 1);
        strcpy(cat, this->str);
        strcat(cat, str);
        return TmpStr(cat);
    }

    TmpStr operator+(const TmpStr& other) {
        SCOPE_EXIT_ROLLBACK;
        char* str = other.str;
        int len = (int)strlen(str);
        char* cat = (char*) tmpAlloc.alloc(len + length + 1);
        strcpy(cat, this->str);
        strcat(cat, str);
        return TmpStr(cat);
    }

    operator const char*() {
        return str;
    }

    operator char*() {
        return str;
    }

    const char* c_str() {
        return str;
    }
};











#endif
