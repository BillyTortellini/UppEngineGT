#include "platformSpecific.cpp"
#include "..\platform.hpp"
#include "..\datatypes.hpp"

#include <windows.h>
#include <windowsx.h>
#include <cstring>
#include <cstdio>

// GLOBALS
GameState gameState = {};

// Rendering
HDC deviceContext;
BITMAPINFO bitmapInfo;
WindowState actualWinState;

// Input
byte keyTranslationTable[NUM_KEYS];

// Debug printing
HANDLE consoleOutHandle;
HANDLE consoleInHandle;

// Prototypes
void resizeVideoBuffer();
void initKeyTranslationTable();

void debugPrint(const char* str) {
    DWORD out;
    WriteConsole(consoleOutHandle, str, strlen(str), &out, NULL);
}

void debugRead(char* buffer, uint32 size) {
    CONSOLE_READCONSOLE_CONTROL c;
    c.nLength = sizeof(c);
    c.nInitialChars = 0;
    c.dwCtrlWakeupMask = '\n';
    c.dwControlKeyState = 0;

    DWORD actuallyRead;
    ReadConsole(consoleInHandle, buffer, size, &actuallyRead, &c);
}

void debugWaitForConsoleInput() {
    char buf[256];
    debugRead(buf, sizeof(buf));
}

#define debugPrintf(format, ...) { \
        char buf[256]; \
        snprintf(buf, 256, format, ##__VA_ARGS__); \
        debugPrint(buf); \
    };

void initKeyTranslationTable() 
{
    keyTranslationTable['A'] = KEY_A; 
    keyTranslationTable['B'] = KEY_B; 
    keyTranslationTable['C'] = KEY_C; 
    keyTranslationTable['D'] = KEY_D; 
    keyTranslationTable['E'] = KEY_E; 
    keyTranslationTable['F'] = KEY_F; 
    keyTranslationTable['G'] = KEY_G; 
    keyTranslationTable['H'] = KEY_H; 
    keyTranslationTable['I'] = KEY_I; 
    keyTranslationTable['J'] = KEY_J; 
    keyTranslationTable['K'] = KEY_K; 
    keyTranslationTable['L'] = KEY_L; 
    keyTranslationTable['M'] = KEY_M; 
    keyTranslationTable['N'] = KEY_N; 
    keyTranslationTable['O'] = KEY_O; 
    keyTranslationTable['P'] = KEY_P; 
    keyTranslationTable['Q'] = KEY_Q; 
    keyTranslationTable['R'] = KEY_R; 
    keyTranslationTable['S'] = KEY_S; 
    keyTranslationTable['T'] = KEY_T; 
    keyTranslationTable['U'] = KEY_U; 
    keyTranslationTable['V'] = KEY_V; 
    keyTranslationTable['W'] = KEY_W; 
    keyTranslationTable['X'] = KEY_X; 
    keyTranslationTable['Y'] = KEY_Y; 
    keyTranslationTable['Z'] = KEY_Z; 

    keyTranslationTable['1'] = KEY_1; 
    keyTranslationTable['2'] = KEY_2; 
    keyTranslationTable['3'] = KEY_3; 
    keyTranslationTable['4'] = KEY_4; 
    keyTranslationTable['5'] = KEY_5; 
    keyTranslationTable['6'] = KEY_6; 
    keyTranslationTable['7'] = KEY_7; 
    keyTranslationTable['8'] = KEY_8; 
    keyTranslationTable['9'] = KEY_9; 
    keyTranslationTable['0'] = KEY_0; 

    keyTranslationTable[VK_F1] = KEY_F1 ; 
    keyTranslationTable[VK_F2] = KEY_F2 ; 
    keyTranslationTable[VK_F3] = KEY_F3 ; 
    keyTranslationTable[VK_F4] = KEY_F4 ; 
    keyTranslationTable[VK_F5] = KEY_F5 ; 
    keyTranslationTable[VK_F6] = KEY_F6 ; 
    keyTranslationTable[VK_F7] = KEY_F7 ; 
    keyTranslationTable[VK_F8] = KEY_F8 ; 
    keyTranslationTable[VK_F9] = KEY_F9 ; 
    keyTranslationTable[VK_F10] = KEY_F10; 
    keyTranslationTable[VK_F11] = KEY_F11; 
    keyTranslationTable[VK_F12] = KEY_F12; 

    keyTranslationTable[VK_RETURN] = KEY_RETURN;
    keyTranslationTable[VK_ESCAPE] = KEY_ESCAPE;
    keyTranslationTable[VK_BACK] = KEY_BACKSPACE;
    keyTranslationTable[VK_TAB] = KEY_TAB;
    keyTranslationTable[VK_SPACE] = KEY_SPACE;

    keyTranslationTable[VK_LCONTROL] = KEY_LCTRL;
    keyTranslationTable[VK_LSHIFT] = KEY_LSHIFT;
    //keyTranslationTable[] = KEY_LALT;
    keyTranslationTable[VK_RCONTROL] = KEY_RCTRL;
    keyTranslationTable[VK_RSHIFT] = KEY_RSHIFT;
    //keyTranslationTable[] = KEY_RALT;
}

void resetInputState() {
    memset(&gameState.input.keyPressed, 0, NUM_KEYS);
    memset(&gameState.input.mousePressed, 0, NUM_MOUSE_KEYS);
    gameState.input.deltaX = 0;
    gameState.input.deltaY = 0;
    gameState.input.mouseWheel = 0;
    gameState.windowState.wasResized = false;
}

LRESULT windowProc(HWND hwnd, UINT msgType, WPARAM wParam, LPARAM lParam)
{
    switch (msgType)
    {
        case WM_CLOSE: // Window must not close after wm_close
            debugPrint("WM_CLOSE\n");
            DestroyWindow(hwnd);
            return 0;
            break;

        case WM_DESTROY: // Window will close after wm_destroy
            debugPrint("WM_DESTORY\n");
            PostQuitMessage(0);
            return 0;
            break;

        case WM_PAINT: // Some part of the window needs to be repainted
            break;

        // Keyboard input
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            {
            int msgType = wParam;
            int repeatCount = lParam & 0xFFFF;
            if (gameState.input.keyDown[keyTranslationTable[msgType]] == false &&
                repeatCount == 1) {
                gameState.input.keyPressed[keyTranslationTable[msgType]]++;
            }
            gameState.input.keyDown[keyTranslationTable[msgType]] = true;
            break;
            }

        case WM_KEYUP:
        case WM_SYSKEYUP:
            {
            int msgType = wParam;
            gameState.input.keyDown[keyTranslationTable[msgType]] = false;
            break;
            }

            // Mouse Input
        case WM_LBUTTONDOWN:
            gameState.input.mouseDown[MOUSE_LEFT] = true;
            gameState.input.mousePressed[MOUSE_LEFT]++;
            return 0;
        case WM_LBUTTONUP:
            gameState.input.mouseDown[MOUSE_LEFT] = false;
            return 0;
        case WM_MBUTTONDOWN:
            gameState.input.mouseDown[MOUSE_MIDDLE] = true;
            gameState.input.mousePressed[MOUSE_MIDDLE]++;
            return 0;
        case WM_MBUTTONUP:
            gameState.input.mouseDown[MOUSE_MIDDLE] = false;
            return 0;
        case WM_RBUTTONDOWN:
            gameState.input.mouseDown[MOUSE_RIGHT] = true;
            gameState.input.mousePressed[MOUSE_RIGHT]++;
            return 0;
        case WM_RBUTTONUP:
            gameState.input.mouseDown[MOUSE_RIGHT] = false;
            return 0;
        case WM_MOUSEWHEEL: 
            {
                int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
                gameState.input.mouseWheel += zDelta / ((float)WHEEL_DELTA);
                return 0;
            }
        case WM_MOUSEMOVE: 
            {
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                gameState.input.deltaX += (x - gameState.input.mouseX);
                gameState.input.deltaY += (y - gameState.input.mouseY);
                gameState.input.mouseX  = x;
                gameState.input.mouseY  = y;
                return 0;
            }

        case WM_SIZE:
            {
                RECT r;
                GetClientRect(hwnd, &r);
                if (actualWinState.width != r.right ||
                        actualWinState.height != r.bottom)  
                {
                    actualWinState.width = r.right;
                    actualWinState.height = r.bottom;
                    gameState.windowState.width = r.right;
                    gameState.windowState.height = r.bottom;
                    gameState.windowState.wasResized = true;
                    resizeVideoBuffer();
                }
                return 0;
            }
    }

    return DefWindowProc(hwnd, msgType, wParam, lParam);
}

void resizeVideoBuffer() 
{
    debugPrintf("ResizeVideoBuffer w/h: %d/%d\n", actualWinState.width, actualWinState.height); 
    if (actualWinState.width == 0 || actualWinState.height == 0) {
        return;
    }

    VideoData* videoData = &gameState.videoData;
    videoData->width = actualWinState.width;
    videoData->height = actualWinState.height;

    if (videoData->pixels != nullptr) {
        // Deallocate memory
        VirtualFree(videoData->pixels, 0, MEM_RELEASE);
    }

    // Alloc memory
    int bytesPerPixel = sizeof(Pixel);
    int imgSize = bytesPerPixel * videoData->width * videoData->height;
    videoData->pixels = (Pixel*) VirtualAlloc(NULL, imgSize, MEM_COMMIT, PAGE_READWRITE);

    bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo);
    bitmapInfo.bmiHeader.biWidth = videoData->width;
    bitmapInfo.bmiHeader.biHeight = videoData->height;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = sizeof(Pixel) * 8;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;
    bitmapInfo.bmiHeader.biSizeImage = imgSize;
    bitmapInfo.bmiHeader.biXPelsPerMeter = 0;
    bitmapInfo.bmiHeader.biYPelsPerMeter = 0;
    bitmapInfo.bmiHeader.biClrUsed = 0;
    bitmapInfo.bmiHeader.biClrImportant = 0;

    memset(videoData->pixels, 0, imgSize);
}

void renderToWindow(HDC deviceContext, BITMAPINFO* bitmapInfo, VideoData* videoData) 
{
    int width = videoData->width;
    int height = videoData->height;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            videoData->pixels[x + y * width].r = 0;
            videoData->pixels[x + y * width].g = 255;
            videoData->pixels[x + y * width].b = 255;
        }
    }

    StretchDIBits (
            deviceContext,
            0, 0, width, height,
            0, 0, width, height,
            videoData->pixels, 
            bitmapInfo,
            DIB_RGB_COLORS,
            SRCCOPY);
}

void gameTick() 
{
    debugPrintf("GameTick!\n");
    Input& input = gameState.input;
    if (input.deltaX != 0 ||
        input.deltaY != 0) {
        debugPrintf("Mouse X/Y: %d/%d, delta: %d/%d \n", input.mouseX, input.mouseY, input.deltaX, input.deltaY);
    }
    if (input.mouseWheel != 0) {
        debugPrintf("MouseWheel: %f\n", input.mouseWheel);
    }
    if (input.keyPressed[KEY_F5]) {
        bool& d = gameState.windowState.continuousDraw;
        d = !d;
        debugPrintf("Continuous draw switched to %s\n", d == true ? "TRUE" : "FALSE");
    }
    if (gameState.input.mousePressed[MOUSE_LEFT]) {
        debugPrint("Mouse Left pressed\n");
        debugPrintf("MB_RIGHT: %d\n", MOUSE_RIGHT);
    }
    if (gameState.input.mousePressed[MOUSE_RIGHT]) {
        debugPrint("Mouse RIGHT pressed\n");
    }
    if (gameState.input.mousePressed[MOUSE_MIDDLE]) {
        debugPrint("Mouse MIDDLE pressed\n");
    }
    if (gameState.input.keyPressed[KEY_ESCAPE]) {
        gameState.windowState.quit = true;
    }
    if (gameState.input.keyPressed[KEY_A]) {
        debugPrint("A pressed\n");
    }
    if (gameState.input.keyPressed[KEY_D]) {
        debugPrint("D pressed\n");
    }
    if (gameState.input.keyPressed[KEY_TAB]) {
        debugPrint("TAB pressed\n");
    }
}

#define CHECK_VALID(cond, msg) if (!(cond)) {debugPrint(msg); return -1;}
int WINAPI WinMain(HINSTANCE instance, HINSTANCE unused, PSTR cmdLine, int cmdShow)
{
    memset(&gameState, 0, sizeof(GameState));
    initKeyTranslationTable();

    // Initialize GameState
    VideoData* videoData = &gameState.videoData;
    {
        Memory* mem = &gameState.memory;
        mem->size = 1024L * 1024L * 1024L * 4L;
        mem->memory = (byte*) VirtualAlloc(NULL, mem->size, MEM_COMMIT, PAGE_READWRITE);
    }

    // Initialize Debug Console
    {
        if (AllocConsole() == NULL) {
            return -1;
        }
        consoleOutHandle= GetStdHandle(STD_OUTPUT_HANDLE);
        consoleInHandle = GetStdHandle(STD_INPUT_HANDLE);
        if (consoleOutHandle == INVALID_HANDLE_VALUE || 
                consoleInHandle == INVALID_HANDLE_VALUE) {
            return -1;
        }
    }

    // Initialize hwnd
    HWND hwnd;
    WindowState& desiredWinState = gameState.windowState;
    {
        const char* CLASS_NAME = "UppEngineGT";
        WNDCLASS wc = {};
        wc.style = CS_HREDRAW  | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = &windowProc;
        wc.cbClsExtra = 0;
        wc.hInstance = instance;
        wc.hIcon = NULL;
        wc.hCursor = NULL;
        wc.hbrBackground = NULL;
        wc.lpszMenuName = NULL;
        wc.lpszClassName = CLASS_NAME;

        if (RegisterClass(&wc) == 0) {
            debugPrint("Register class failed!\n");
            return -1;
        }

        hwnd = CreateWindowEx(
                WS_EX_OVERLAPPEDWINDOW,
                CLASS_NAME,
                CLASS_NAME,
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT,
                CW_USEDEFAULT, CW_USEDEFAULT,
                NULL,
                NULL,
                instance,
                NULL);
        if (hwnd == NULL) {
            debugPrint("CreateWindowEX failed!\n");
            return -1;
        }
        ShowWindow(hwnd, cmdShow);

        // Init winState
        actualWinState.fullscreen = false;
        actualWinState.minimized = false;
        actualWinState.continuousDraw = false;
        memcpy(&desiredWinState, &actualWinState, sizeof(WindowState));
    }

    // Initialize Drawing with GDI
    {
        deviceContext = GetDC(hwnd);
        CHECK_VALID(deviceContext != 0, "GetDC failed\n");

        resizeVideoBuffer(); // Initializes video memory
    }

    // MSG Loop
    MSG msg;
    bool quit = false;
    while (!quit) 
    {
        if (actualWinState.continuousDraw) 
        {
            // Process all available messages
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0) 
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);

                if (msg.message == WM_QUIT) {
                    quit = true;
                }
            }

            // Handle requests
            if (desiredWinState.quit) {
                DestroyWindow(hwnd);
            }
            if (actualWinState.fullscreen != desiredWinState.fullscreen) {
                // Switch to fullscreen TODO
                desiredWinState.fullscreen = actualWinState.fullscreen;
            }
            if (actualWinState.minimized != desiredWinState.minimized) {
                // Switch to minimized TODO
                desiredWinState.minimized = actualWinState.minimized;
            }
            actualWinState.continuousDraw = desiredWinState.continuousDraw;

            // GameTick...
            gameTick();
            resetInputState();
            renderToWindow(deviceContext, &bitmapInfo, &gameState.videoData);
        }
        else 
        {
            // Get one message and block afterwards
            quit = (GetMessage(&msg, NULL, 0, 0) == 0);
            if (quit) { continue; }
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            // Get remaining messages
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0) 
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);

                if (msg.message == WM_QUIT) {
                    quit = true;
                }
            }

            // React to program changes
            if (desiredWinState.quit) {
                DestroyWindow(hwnd);
            }
            actualWinState.continuousDraw = desiredWinState.continuousDraw;

            // GameTick...
            gameTick();
            resetInputState();
            renderToWindow(deviceContext, &bitmapInfo, &gameState.videoData);
        }
    }

    debugPrint("Program exit\n");
    debugWaitForConsoleInput();

    return 0;
}
