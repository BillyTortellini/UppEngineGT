#ifndef __DEBUGGING_TOOLS_HPP__
#define __DEBUGGING_TOOLS_HPP__

// -----------------------
// --- DEBUGGING TOOLS ---
// -----------------------
// This file contains 2 DECLARATIONS of debugging functionality
// that can be set by the user of this library. If they are not set, stderr and a 
// simple wait for enter will be used.
// By defining the MACRO _NO_DEBUG_FUNCS, the debugging utility will be turned off.

// Debugging functionality includes:
//  * Invalid path (Should stop the program and show a message)
//  * Assertions (Tests a given condition and stops the program if it is met)
//  * Logging (Prints information, but does not stop the program)

// Standard library includes
#include <stdio.h> // For printf (Used in default functions)
#include <iostream> // For cin.get() (Used in default_invalid_path)
#include <stdlib.h> // Used for exit (default_invalid_path)
#include <stdarg.h> // Used for printf variadic argument list

// DECLARATIONS
typedef void (*loggFunc)(const char* msg);
typedef void (*invalid_pathFunc)(const char* msg);


#ifdef _NO_DEBUG_FUNCS
// Path if debugging should be DISABLED

#define invalid_path(x)
#define assert(x, y, ...)
#define logg(x)
#define loggf(x, ...)
#define setDebugFunctions(x, y)

#else
// Path if debugging should be ENABLED
void default_logg(const char* msg) {
    fprintf(stderr, "%s", msg);
}

void default_invalid_path(const char* msg) {
    default_logg("INVALID_CODE_PATH, msg:\n");
    default_logg(msg);
    while (std::cin.get() != '\n') {}
    exit(EXIT_FAILURE);
}

loggFunc logg = &default_logg;
invalid_pathFunc invalid_path = &default_invalid_path;

void setDebugFunctions(loggFunc l, invalid_pathFunc i) {
    logg = l;
    invalid_path = i;
}

// DEFINITIONS
char __loggf_buffer[2048];
void loggf(const char* format, ...) 
{
    va_list argptr;
    va_start(argptr, format);
    vsnprintf(__loggf_buffer, 2048, format, argptr);
    va_end(argptr);

    logg(__loggf_buffer);
}

void assert(bool condition, const char* msg, ...)
{
    if (!condition)
    {
        va_list argptr;
        va_start(argptr, msg);
        vsnprintf(__loggf_buffer, 2048, msg, argptr);
        va_end(argptr);

        invalid_path(__loggf_buffer);
    }
}

#endif



#endif
