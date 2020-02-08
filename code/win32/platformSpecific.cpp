#ifndef _PLATFORM_SPECIFIC_
#define _PLATFORM_SPECIFIC_

#include "../datatypes.hpp"

#define DECLARE_EXPORT __declspec(dllexport)

struct Pixel
{
    byte a;
    byte b;
    byte r;
    byte g;
};

#endif
