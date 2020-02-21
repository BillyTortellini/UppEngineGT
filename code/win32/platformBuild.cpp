// PLATFORM BUILD FOR WINDOWS OS

// Includes for the game bould
// STILL DONT KNOW WHY I HAVE TO INCLUDE THIS FIRST
#include "uppLib.hpp"

// Includes so that opengl functions are defined
#include <windows.h>
#include <GL/GL.h>
#include <GL/glext.h>
#include <GL/wglext.h>

// DEBUGGING: Add line number and file name to each assert call
char _assert_line_num_buf[4096];
char _assert_line_helper[4096];
#define assert(x, y, ...) \
{ \
snprintf(_assert_line_num_buf, 4096, (y), __VA_ARGS__); \
snprintf(_assert_line_helper, 4096, "Line: %d, file: %s:\n", __LINE__, __FILE__); \
strncat(_assert_line_helper, _assert_line_num_buf, 4096); \
assert((x), _assert_line_helper); \
} 
// DEBUGGING END

#include "..\platform.hpp"
#include "..\game.hpp"
#include "..\game.cpp"
#include "win32_gameHooks.cpp"
