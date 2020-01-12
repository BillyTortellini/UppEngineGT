#ifndef __GAME_HPP__
#define __GAME_HPP__

#include "datatypes.hpp"

struct Memory
{
    byte* memory;
    uint64 size;
};

struct Input
{
    const byte* keyDown; // If the key is currently down
    const byte* keyPressed; // Contains the count how many times the key was pressed between last and current frame
    const byte* mouseDown;
    const byte* mousePressed;
    int mouseX, mouseY;
};

struct WindowState
{
    int width, height;
    bool fullscreen;
    bool minimized;
    bool quit;
    bool continuousDraw;
    bool wasResized;
};

struct Pixel; // Has to contain bytes r, g, b and a. Is defined by the platform

struct VideoData
{
    Pixel* pixels;
    int width;
    int height;
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

typedef void (*lockSoundFunc)();
typedef void (*unlockSoundFunc)();
typedef uint32 (*getFileSizeFunc)(const char* filename);
typedef void (*loadFileFunc)(byte* memory);
typedef void (*debugPrintFunc)(const char* str);

struct Services
{
    lockSoundFunc lockSound; 
    unlockSoundFunc unlockSound;
    getFileSizeFunc getFileSize;
    loadFileFunc loadFileFunc;
    debugPrintFunc debugPrint;
};

struct GameState
{
    Memory memory;
    Input input;
    Time time;
    VideoData videoData;
    SoundInfo soundInfo;
    WindowState windowState;
    Services services;
};

typedef void (*gameInitFunc)(GameState* state);
typedef void (*gameTickFunc)(GameState* state);
typedef void (*gameShutdownFunc)(GameState* state);
typedef void (*gameAudioFunc)(GameState* state, byte* stream, int length);

typedef enum
{
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
    MB_LEFT = 0,
    MB_RIGHT = 3,
    MB_MIDDLE = 1,
} UPP_MOUSEKEY;

#endif
