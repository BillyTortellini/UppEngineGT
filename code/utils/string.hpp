#ifndef __STRING_HPP__
#define __STRING_HPP__

#include <ctype.h> // for tolower

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












#endif
