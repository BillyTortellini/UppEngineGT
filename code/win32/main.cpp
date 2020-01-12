#include "platformSpecific.cpp"
#include "..\platform.hpp"
#include "..\datatypes.hpp"

#include <windows.h>
#include <cstring>

// GLOBALS
GameState gameState = {};

// Rendering
HDC deviceContext;
BITMAPINFO bitmapInfo;

// Debug printing
HANDLE consoleOutHandle;
HANDLE consoleInHandle;

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
    }

    return DefWindowProc(hwnd, msgType, wParam, lParam);
}

void resizeVideoBuffer() 
{
    VideoData* videoData = &gameState.videoData;

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

void renderToWindow(HDC deviceContext) {
    int width = gameState.videoData.width;
    int height = gameState.videoData.height;
    StretchDIBits (
            deviceContext,
            0, 0, width, height,
            0, 0, width, height,
            gameState.videoData.pixels, 
            &bitmapInfo,
            DIB_RGB_COLORS,
            SRCCOPY);
}

#define CHECK_VALID(cond, msg) if (!(cond)) {debugPrint(msg); return -1;}
int WINAPI WinMain(HINSTANCE instance, HINSTANCE unused, PSTR cmdLine, int cmdShow)
{
    // Initialize GameState
    WindowState actualWinState;
    WindowState* desiredWinState = &gameState.windowState;
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

        // Get Window Size
        RECT r;
        GetClientRect(hwnd, &r);
        videoData->width = r.right;
        videoData->height = r.bottom;
        actualWinState.width = r.right;
        actualWinState.height = r.bottom;

        // Init winState
        actualWinState.fullscreen = false;
        actualWinState.minimized = false;
        actualWinState.continuousDraw = true;
        memcpy(desiredWinState, &actualWinState, sizeof(WindowState));
    }

    // Initialize Drawing with GDI
    {
        HDC deviceContext = GetDC(hwnd);
        CHECK_VALID(deviceContext != 0, "GetDC failed\n");

        resizeVideoBuffer(); // Initializes video memory
    }

    // MSG Loop
    MSG msg;
    bool quit = false;
    while (!quit) 
    {
        debugPrint("One Loop");
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
            if (desiredWinState->quit) {
                quit = true;
            }
            if (actualWinState.fullscreen != desiredWinState->fullscreen) {
                // Switch to fullscreen TODO
                desiredWinState->fullscreen = actualWinState.fullscreen;
            }
            if (actualWinState.minimized != desiredWinState->minimized) {
                // Switch to minimized TODO
                desiredWinState->minimized = actualWinState.minimized;
            }
            actualWinState.continuousDraw = desiredWinState->continuousDraw;

            // GameTick...
        }
        else 
        {
            // Get one message and block afterwards
            quit = (GetMessage(&msg, NULL, 0, 0) != 0);

            // GameTick...
        }
    }

    debugPrint("Program exit\n");
    debugWaitForConsoleInput();

    return 0;
}
