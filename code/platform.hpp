#ifndef __PLATFORM_HPP__
#define __PLATFORM_HPP__

#include "uppLib.hpp"

// DEFINES
#define NUM_KEYS 256
#define NUM_MOUSE_KEYS 3

// PLATFORM CALLBACKS
typedef int ListenerToken;
typedef void (*listenerCallbackFunc)(const char* filename, void* userData);
ListenerToken createFileListener(const char* path, listenerCallbackFunc callback, void* userData);
void deleteFileListener(ListenerToken token);

// PLATFORM STRUCTS
struct Input
{
    byte keyDown[NUM_KEYS]; // If the key is currently down
    byte keyPressed[NUM_KEYS]; // Contains the count how many times the key was pressed between last and current frame
    byte mouseDown[NUM_MOUSE_KEYS];
    byte mousePressed[NUM_MOUSE_KEYS];
    float mouseWheel;
    int mouseX, mouseY; // TODO: Eventually floating point
    int deltaX, deltaY;
};

struct WindowState
{
    // These variables have no effect when being set
    // They also will be reset every frame
    bool wasResized; 
    bool wasMoved; 
    bool wasMinimized;

    // These variables can be set
    bool fullscreen;
    bool quit;
    bool hideCursor;
    bool minimized;

    int width; // Client size
    int height; // Client size
    int x, y;
};

struct RenderOptions
{
    int fps; // How often gameTick should be called
    bool vsync; 
    bool continuousDraw; // If true, SwapBuffers will be called after 
    bool redraw; // If continousDraw is false, window will only be refreshed after redraw is set to true
};

struct Time
{
    double now; // In seconds since game start
    double tslf; // In seconds
};

struct SoundInfo
{
    int sampleFreq;
    int numChannels;
    int bytePerSample;
    int sampleDelay;
};

struct GameState
{
    Blk memory;
    Input input;
    Time time;
    RenderOptions renderOptions;
    SoundInfo soundInfo;
    WindowState windowState;
};

typedef enum
{
    KEY_UNASSIGNED = 0, // Used in keyboard translation tables to indicate that no key mapping exists
    KEY_A = 4,
    KEY_B = 5,
    KEY_C = 6,
    KEY_D = 7,
    KEY_E = 8,
    KEY_F = 9,
    KEY_G = 10,
    KEY_H = 11,
    KEY_I = 12,
    KEY_J = 13,
    KEY_K = 14,
    KEY_L = 15,
    KEY_M = 16,
    KEY_N = 17,
    KEY_O = 18,
    KEY_P = 19,
    KEY_Q = 20,
    KEY_R = 21,
    KEY_S = 22,
    KEY_T = 23,
    KEY_U = 24,
    KEY_V = 25,
    KEY_W = 26,
    KEY_X = 27,
    KEY_Y = 28,
    KEY_Z = 29,

    KEY_1 = 30,
    KEY_2 = 31,
    KEY_3 = 32,
    KEY_4 = 33,
    KEY_5 = 34,
    KEY_6 = 35,
    KEY_7 = 36,
    KEY_8 = 37,
    KEY_9 = 38,
    KEY_0 = 39,

    KEY_RETURN = 40,
    KEY_ESCAPE = 41,
    KEY_BACKSPACE = 42,
    KEY_TAB = 43,
    KEY_SPACE = 44,
    KEY_SHIFT = 45,
    KEY_CTRL = 46,
    KEY_ALT = 47,

    KEY_F1 = 58,
    KEY_F2 = 59,
    KEY_F3 = 60,
    KEY_F4 = 61,
    KEY_F5 = 62,
    KEY_F6 = 63,
    KEY_F7 = 64,
    KEY_F8 = 65,
    KEY_F9 = 66,
    KEY_F10 = 67,
    KEY_F11 = 68,
    KEY_F12 = 69,

    KEY_LCTRL = 224,
    KEY_LSHIFT = 225,
    KEY_LALT = 226, /**< alt, option */
    KEY_RCTRL = 228,
    KEY_RSHIFT = 229,
    KEY_RALT = 230, /**< alt gr, option */
} UPP_KEYCODE;

typedef enum
{
    MOUSE_LEFT = 0,
    MOUSE_RIGHT = 1,
    MOUSE_MIDDLE = 2,
} UPP_MOUSEKEY;

#endif
