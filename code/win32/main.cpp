#include "platformSpecific.cpp"
#include "..\platform.hpp"
#include "..\datatypes.hpp"
#include "..\umath.hpp"
#include "..\scopedExit.hpp"

#include <windows.h>
#include <windowsx.h>
#include <wingdi.h>
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <initializer_list>
#include <intrin.h>
#include <GL/GL.h>
#include <GL/glext.h>
#include <GL/wglext.h>

// Prototypes
char* load_text_file(const char* filepath);
void unload_text_file(char* data);

// Define debug macro and prototypes so that they can be used in other win32 files
#undef INVALID_CODE_PATH
#define INVALID_CODE_PATH \
{ \
    char invalidBuffer[2048]; \
    snprintf(invalidBuffer, 2048, "Invalid code path: %s\tLine #%d", __FILE__, __LINE__); \
    MessageBox(NULL, invalidBuffer, "ERROR", MB_OK); \
    __debugbreak(); \
    exit(-1); \
}

#undef ASSERT
#define ASSERT(x) \
if (!(x)) { \
    char assertBuf[2048]; \
    snprintf(assertBuf, 2048, "Assertion failed:\nFile: %s\tLine #%d", __FILE__, __LINE__); \
    MessageBox(hwnd, assertBuf, "ERROR", MB_OK); \
    __debugbreak(); \
    exit(-1); \
}

char _log_buffer[4096];
#undef log
#define log(format, ...) { \
        snprintf(_log_buffer, 4096, format, ##__VA_ARGS__); \
        debugPrint(_log_buffer); \
};

#undef debugPrintf
#define debugPrintf(format, ...) { \
        char buf[1024]; \
        snprintf(buf, 1024, format, ##__VA_ARGS__); \
        debugPrint(buf); \
};

void debugPrint(const char* str);
void debugWaitForConsoleInput();

// Undefine near and far macros that are defined in the windows headers
#undef near
#undef far

#include "../allocators.hpp"
#include "win32_glFunctions.hpp"
#include "../string.hpp"
#include "../renderer.hpp"

// GLOBALS
GameState gameState = {};

// Rendering
HDC deviceContext;
BITMAPINFO bitmapInfo;
WindowState actualWinState;

// Window State
RECT savedWindowPos;
HWND hwnd = NULL;

// Input
byte keyTranslationTable[NUM_KEYS];

// Timing
int64 performanceFrequency;

// Debug printing
HANDLE consoleOutHandle;
HANDLE consoleInHandle;

// Prototypes
void resizeVideoBuffer();
void initKeyTranslationTable();

void debugPrint(const char* str) {
    DWORD out;
    WriteConsole(consoleOutHandle, str, (DWORD)strlen(str), &out, NULL);
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

bool initDebugConsole() {
    if (AllocConsole() == NULL) {
        return false;
    }
    consoleOutHandle= GetStdHandle(STD_OUTPUT_HANDLE);
    consoleInHandle = GetStdHandle(STD_INPUT_HANDLE);
    if (consoleOutHandle == INVALID_HANDLE_VALUE || 
            consoleInHandle == INVALID_HANDLE_VALUE) {
        return false;
    }
    return true;
}

void printLastError() {
    DWORD error = GetLastError();
    char* msgBuffer;
    FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR) &msgBuffer,
            0, NULL);
    debugPrintf("Last error code: %d, Error:\n%s\n", error, msgBuffer);
    debugWaitForConsoleInput();
}

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

void resetInputState() 
{
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
            int key = (int)wParam;
            int repeatCount = lParam & 0xFFFF;
            if (gameState.input.keyDown[keyTranslationTable[key]] == false &&
                repeatCount == 1) {
                gameState.input.keyPressed[keyTranslationTable[key]]++;
            }
            gameState.input.keyDown[keyTranslationTable[key]] = true;
            break;
            }

        case WM_KEYUP:
        case WM_SYSKEYUP:
            {
            int key = (int)wParam;
            gameState.input.keyDown[keyTranslationTable[key]] = false;
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
                if (wParam == SIZE_MINIMIZED) {
                    actualWinState.minimized = true;
                    gameState.windowState.minimized = true;
                    return 0;
                }
                RECT r;
                GetClientRect(hwnd, &r);
                if (actualWinState.width != r.right ||
                        actualWinState.height != r.bottom)  
                {
                    actualWinState.minimized = false;
                    gameState.windowState.minimized = false;
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

// Prototype
void requestDirectoryChanges();

HANDLE fileHandle;
char filename[256];
char path[512];
void initFileListener(char* pathToFile) 
{
    int lastSlash = -15;
    int len = (int)strlen(pathToFile);
    for (int i = len-1; i >= 0; i--) {
        if (pathToFile[i] == '/' || pathToFile[i] == '\\') {
            lastSlash = i;
            break;
        }
    }

    if (lastSlash == -15) {
        strcpy(filename, pathToFile);
        strcpy(path, ".\\");
    }
    else {
        strcpy(filename, &pathToFile[lastSlash+1]);
        strcpy(path, pathToFile);
        path[lastSlash] = '\0';
    }

    fileHandle = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL); 
    if (fileHandle == INVALID_HANDLE_VALUE) {
        printLastError();
        debugPrint("FindFirstChange.. failed, invalid handle!\n");
        debugWaitForConsoleInput();
    }

    requestDirectoryChanges();
}

const int notifyBufferSize = 2;
DWORD notifyBuffer[2048];
OVERLAPPED overlapped = {};
void requestDirectoryChanges() 
{
    bool success = ReadDirectoryChangesW(
            fileHandle,
            notifyBuffer,
            sizeof(DWORD) * 2048,
            FALSE,
            FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
            NULL,
            &overlapped,
            NULL);

    if (!success) {
        debugPrint("ReadDirectoryChanges failed! No more file changes will be noticed.\n");
        printLastError();
    }
}

bool checkFileChanged() 
{
    bool found = false;

    DWORD bytesReturned;
    bool result = GetOverlappedResult(fileHandle, &overlapped, &bytesReturned, false);
    if (!result) 
    {
        DWORD error = GetLastError();
        if (error == ERROR_IO_INCOMPLETE) {
            //debugPrint("GetOverlappedResult IO_INCOMPLETE\n");
            return false;
        }
        printLastError();
    }
    else 
    {
        int notificationCount = bytesReturned / sizeof(_FILE_NOTIFY_INFORMATION);
        bool quit = notificationCount == 0;
        if (quit) {
            debugPrintf("Notification count was zero, should not happen\n");
            quit = true;
        }
        int nextEntryOffset = 0;
        // Loop over all file_notifications
        while(!quit)
        {
            _FILE_NOTIFY_INFORMATION& info = *((_FILE_NOTIFY_INFORMATION*)((byte*)notifyBuffer + nextEntryOffset));
            if (info.FileNameLength == 0) {
                debugPrintf("FileNameLength was 0, should not happen\n");
                quit = true;
                continue;
            }
            char changedName[256];
            int res = WideCharToMultiByte(CP_UTF8, NULL, info.FileName, info.FileNameLength, changedName, 256, NULL, NULL);
            if (res == 0) { 
                debugPrintf("WideCharToMultibyte failed!\n"); 
                printLastError();
                quit = true;
                continue;
            }
            changedName[res/2] = 0;

            /*debugPrintf("File changed: %s, ", changedName);
            switch(info.Action)
            {
                case FILE_ACTION_ADDED:
                    debugPrintf("FILE_ACTION_ADDED\n");
                    break;
                case FILE_ACTION_REMOVED:
                    debugPrintf("FILE_ACTION_REMOVED\n");
                    break;
                case FILE_ACTION_MODIFIED:
                    debugPrintf("FILE_ACTION_MODIFIED\n");
                    break;
                case FILE_ACTION_RENAMED_OLD_NAME:
                    debugPrintf("FILE_ACTION_RENAMED_OLD_NAME\n");
                    break;
                case FILE_ACTION_RENAMED_NEW_NAME:
                    debugPrintf("FILE_ACTION_RENAMED_NEW_NAME\n");
                    break;
            }*/
            if (strcmp(changedName, filename) == 0) 
            {
                quit = true;
                found = true;
                continue;
            }

            nextEntryOffset += info.NextEntryOffset;
            if (info.NextEntryOffset == 0) break;
        }

        requestDirectoryChanges();
    }

    return found;
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

void unload_text_file(char* data) {
    if (data != NULL) {
        free(data);
    }
}

// Allocates memory for the whole file, then reads the file into the specified memory area
char* load_text_file(const char* filepath) 
{
    FILE* file = fopen(filepath, "rb");
    if (file == NULL) {
        debugPrintf("File %s could not be openend!\n", filepath);
        return nullptr;
    }

    // Get File size
    fseek(file, 0, SEEK_END); 
    int fileSize = (int) ftell(file);
    fseek(file, 0, SEEK_SET); // Put cursor back to start of file

    // Alloc memory for file data
    char *data = (char*) malloc(fileSize + 1);
    
    // Read
    int readSize = (int) fread(data, 1, fileSize, file); 
    if (readSize != fileSize) {
        debugPrintf("fread failed, it returned %d size instead of %d fileSize\n",
                readSize, fileSize);
        unload_text_file(data);
        fclose(file);
        return nullptr;
    }

    // Add null terminator
    data[fileSize] = '\0';
    fclose(file);

    return data;
}

gameInitFunc gameInit;
gameTickFunc gameTick;
gameShutdownFunc gameShutdown;
gameAudioFunc gameAudio;

void fallbackGameInit(GameState* g) {}
void fallbackGameTick(GameState* g) {}
void fallbackGameShutdown(GameState* g) {}
void fallbackGameAudio(GameState* g, byte* stream, int length) {}

void setToFallback()
{
    gameInit = &fallbackGameInit;
    gameTick = &fallbackGameTick;
    gameShutdown = &fallbackGameShutdown;
    gameAudio = &fallbackGameAudio;
}

HMODULE gameLibrary = NULL;
void loadGameFunctions()
{
    debugPrintf("LOAD GAME FUNCTIONS\n");
    if (gameLibrary != NULL) {
        FreeLibrary(gameLibrary);
    }

    bool res = CopyFile("build\\game_tmp.dll", "build\\game_inUse.dll", FALSE);
    if (!res) {
        debugPrintf("Could not copy file game_tmp.dll\n");
        setToFallback();
        return;
    }

    gameLibrary = LoadLibrary("build\\game_inUse.dll");
    if (gameLibrary == NULL) {
        debugPrintf("LoadGameFunctions failed: loadlibrary failed\n");
        setToFallback();
        printLastError();
        return;
    }

    gameInit = (gameInitFunc) GetProcAddress(gameLibrary, "gameInit");
    gameTick = (gameTickFunc) GetProcAddress(gameLibrary, "gameTick");
    gameShutdown = (gameShutdownFunc) GetProcAddress(gameLibrary, "gameShutdown");
    gameAudio = (gameAudioFunc) GetProcAddress(gameLibrary, "gameAudio");
    if (gameInit == NULL || gameTick == NULL || gameShutdown == NULL || gameAudio == NULL)
    {
        debugPrintf("GetProcAddress failed: One or more functions were not found in dll.\n");
        setToFallback();
        return;
    }
}

void initDynamicLoading() 
{
    gameInit = &fallbackGameInit;
    gameTick = &fallbackGameTick;
    gameShutdown = &fallbackGameShutdown;
    gameAudio = &fallbackGameAudio;
    loadGameFunctions();
}

void renderToWindow(HDC deviceContext, BITMAPINFO* bitmapInfo, VideoData* videoData) 
{
    int width = videoData->width;
    int height = videoData->height;
    StretchDIBits (
            deviceContext,
            0, 0, width, height,
            0, 0, width, height,
            videoData->pixels, 
            bitmapInfo,
            DIB_RGB_COLORS,
            SRCCOPY);
}

void initTiming() {
    bool res = QueryPerformanceFrequency((LARGE_INTEGER*) &performanceFrequency);    
    if (!res) {
        printLastError();
    }
}

int64 currentTick() {
    return __rdtsc();
}

double currentTime()
{
    int64 now = 0;
    QueryPerformanceCounter((LARGE_INTEGER*) &now);
    return (double) now / performanceFrequency;
}

void sleepUntil(double until)
{
    double now = currentTime();
    double diff = until - now;
    if (diff <= 0.0) return;

    // Calcuate ms for sleep
    int ms = (int)(diff*1000);
    // To make sure that time is as accurate as possible, we sleep one ms less and do busy waiting
    ms -= 1;

    if (ms > 0) {
        timeBeginPeriod(1);
        Sleep(ms);
        timeEndPeriod(1);
    }

    do {} while (currentTime() < until);
}

void initAudio()
{
    HMODULE directSoundLib = LoadLibrary("dsound.dll");
    if (directSoundLib)
    {
    }
    debugPrintf("asdf audio\n");
    debugWaitForConsoleInput();
}

void sleepFor(double seconds) {
    double start = currentTime();
    sleepUntil(start + seconds);
}

void debugGameTick() 
{
    Input& input = gameState.input;
    if (input.deltaX != 0 ||
            input.deltaY != 0) {
        //debugPrintf("Mouse X/Y: %d/%d, delta: %d/%d \n", input.mouseX, input.mouseY, input.deltaX, input.deltaY);
    }
    if (gameState.windowState.wasResized) {
        debugPrintf("GAMETICK: resized to %d/%d\n", gameState.windowState.width, gameState.windowState.height);
    }
    if (input.mouseWheel != 0) {
        debugPrintf("MouseWheel: %f\n", input.mouseWheel);
    }
    if (input.keyPressed[KEY_F5]) {
        bool& d = gameState.windowState.continuousDraw;
        d = !d;
        debugPrintf("Continuous draw switched to %s\n", d == true ? "TRUE" : "FALSE");
    }
    if (input.keyPressed[KEY_F11]) {
        bool& f = gameState.windowState.fullscreen;
        f = !f;
        debugPrintf("Fullscreen toggled to %s\n", f == true ? "TRUE" : "FALSE");
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
    gameState.windowState.continuousDraw = true;
    gameState.windowState.fps = 60;

    render();
}



int windowStyle;
int windowStyleEX;
HCURSOR cursor;
const char* CLASS_NAME = "UppEngineGT";
HINSTANCE globalInstance;
bool initWindow()
{
    windowStyle = WS_OVERLAPPEDWINDOW;
    windowStyleEX = WS_EX_OVERLAPPEDWINDOW;

    // Load cursor
    cursor = LoadCursor(NULL, IDC_ARROW);
    if (cursor == NULL) {
        debugPrint("LoadCursor failed\n");
        printLastError();
        return false;
    }

    WNDCLASS wc = {};
    wc.style = CS_HREDRAW  | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = &windowProc;
    wc.cbClsExtra = 0;
    wc.hInstance = globalInstance;
    wc.hIcon = NULL;
    wc.hCursor = cursor;
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = CLASS_NAME;

    if (RegisterClass(&wc) == 0) {
        debugPrint("Register class failed!\n");
        return false;
    }

    return true;
}

#define CHECK_VALID(cond, msg) if (!(cond)) {debugPrint(msg); return false;}
bool createWindow(bool show)
{
    hwnd = CreateWindowEx(
            windowStyleEX,
            CLASS_NAME,
            CLASS_NAME,
            windowStyle,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            NULL,
            NULL,
            globalInstance,
            NULL);
    if (hwnd == NULL) {
        debugPrint("CreateWindowEX failed!\n");
        return false;
    }

    if (show) {
        ShowWindow(hwnd, SW_SHOW);
    }

    // Init savedWindowPos for toggling fullscreen
    GetWindowRect(hwnd, &savedWindowPos);

    // Init winState
    actualWinState.fullscreen = false;
    actualWinState.minimized = false;
    actualWinState.continuousDraw = false;
    actualWinState.fps = 60;
    WindowState& desiredWinState = gameState.windowState;
    memcpy(&desiredWinState, &actualWinState, sizeof(WindowState));

    // Initialize Drawing with GDI
    deviceContext = GetDC(hwnd);
    CHECK_VALID(deviceContext != 0, "GetDC failed\n");

    resizeVideoBuffer(); // Initializes video memory

    return true;
}

void destroyWindow()
{
    if (hwnd != NULL) {
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
}

PIXELFORMATDESCRIPTOR getDummyDescriptor()
{
    PIXELFORMATDESCRIPTOR pfd;
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits = 0;
    pfd.cRedShift = 0;
    pfd.cGreenBits = 0;
    pfd.cGreenShift = 0;
    pfd.cBlueBits = 0;
    pfd.cBlueShift = 0;
    pfd.cAlphaBits = 0;
    pfd.cAlphaShift = 0;
    pfd.cAccumBits = 0;
    pfd.cAccumRedBits = 0; 
    pfd.cAccumGreenBits = 0;
    pfd.cAccumBlueBits = 0;
    pfd.cAccumAlphaBits = 0;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.cAuxBuffers = 0;
    pfd.iLayerType = 0;
    pfd.bReserved = 0;
    pfd.dwLayerMask = 0;
    pfd.dwVisibleMask = 0;
    pfd.dwDamageMask = 0;
    return pfd;
}

HGLRC glContext;
bool createDummyContext()
{
    PIXELFORMATDESCRIPTOR pfd = getDummyDescriptor();
    int index = ChoosePixelFormat(deviceContext, &pfd);
    if (index == NULL) {
        debugPrintf("ChoosePixelFormat failed\n");
        printLastError();
        return false;
    }

    // Check if the closest pixel format supports what we need
    PIXELFORMATDESCRIPTOR available;
    int ret = DescribePixelFormat(deviceContext, index, sizeof(PIXELFORMATDESCRIPTOR), &available);
    if (ret == NULL) {
        debugPrintf("DescribePixelFormat failed\n");
        printLastError();
        return false;
    }

    // Check if availabe is good enough
    bool success = true;
    // PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    success = success && (available.dwFlags | PFD_DRAW_TO_WINDOW); // Opengl, doublebuffer and draw to windos is necessary
    success = success && (available.dwFlags | PFD_SUPPORT_OPENGL); // Opengl, doublebuffer and draw to windos is necessary
    success = success && (available.dwFlags | PFD_DOUBLEBUFFER); // Opengl, doublebuffer and draw to windos is necessary
    success = success && (available.iPixelType == pfd.iPixelType); // RGBA is necessary

    if (!success) {
        debugPrintf("Available pixelformat does not fullfill the games requirements\n");
        return false;
    }

    // Set pixel format
    if (SetPixelFormat(deviceContext, index, &pfd) == FALSE) {
        debugPrintf("SetPixelFormat failed\n");
        printLastError();
        return false;
    }

    // Create the OpenGL context
    glContext = wglCreateContext(deviceContext);
    if (glContext == NULL) {
        debugPrintf("wglCreateContext failed\n");
        printLastError();
        return false;
    }

    if (wglMakeCurrent(deviceContext, glContext) == FALSE) {
        debugPrintf("wglMakeCurrent failed\n");
        printLastError();
        return false;
    }

    return true;
}

void destroyOpenGLContext()
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(glContext);
}


void customDebugProc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, 
        const GLchar* message, const void* userParam)
{
    // Ignore unecessary warniings
    if (id == 0) return;

    debugPrintf("CustomDebugProc: ERROR ID: %d:, msg: \"%s\"\n", id, message);
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             debugPrintf("Source: API\t"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   debugPrintf("Source: Window System\t"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: debugPrintf("Source: Shader Compiler\t"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     debugPrintf("Source: Third Party\t"); break;
        case GL_DEBUG_SOURCE_APPLICATION:     debugPrintf("Source: Application\t"); break;
        case GL_DEBUG_SOURCE_OTHER:           debugPrintf("Source: Other\t"); break;
    } 

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               debugPrintf("Type: Error\t"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: debugPrintf("Type: Deprecated Behaviour\t"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  debugPrintf("Type: Undefined Behaviour\t"); break; 
        case GL_DEBUG_TYPE_PORTABILITY:         debugPrintf("Type: Portability\t"); break;
        case GL_DEBUG_TYPE_PERFORMANCE:         debugPrintf("Type: Performance\t"); break;
        case GL_DEBUG_TYPE_MARKER:              debugPrintf("Type: Marker\t"); break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          debugPrintf("Type: Push Group\t"); break;
        case GL_DEBUG_TYPE_POP_GROUP:           debugPrintf("Type: Pop Group\t"); break;
        case GL_DEBUG_TYPE_OTHER:               debugPrintf("Type: Other\t"); break;
    } 

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         debugPrintf("Severity: high\t"); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       debugPrintf("Severity: medium\t"); break;
        case GL_DEBUG_SEVERITY_LOW:          debugPrintf("Severity: low\t"); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: debugPrintf("Severity: notification\t"); break;
    }
    debugPrintf("\n");
}

PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
bool createWindowWithOpenGL()
{
    debugPrintf("Init window with OpenGL\n");

    if (!createWindow(false)) {
        debugPrintf("CreateWindow dummy failed\n");
        return false;
    }

    if (!createDummyContext()) {
        debugPrintf("Create dummy context failed\n");
        return false;
    }

    debugPrintf("Dummy context creation worked!\n");
    char* version = (char*)glGetString(GL_VERSION);
    debugPrintf("dummy context version: \"%s\"\n", version);

    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) getAnyGLFuncAddress("wglCreateContextAttribsARB");
    if (wglCreateContextAttribsARB == NULL) {
        debugPrintf("Get andy func failed for createContextAttribs\n");
        return false;
    }

    // Destroy dummy context
    destroyOpenGLContext();

    // Create real context
    int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 5,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    glContext = wglCreateContextAttribsARB(deviceContext, NULL, attribs);
    if (glContext == NULL) {
        debugPrintf("glContext was null!\n");
        debugWaitForConsoleInput();
        return false;
    }

    // Make context current
    if (wglMakeCurrent(deviceContext, glContext) == FALSE) {
        debugPrintf("wglMakeCurrent failed\n");
        printLastError();
        return false;
    }

    debugPrintf("Initializisation version 4.5 worked!!!\n");
    version = (char*)glGetString(GL_VERSION);
    debugPrintf("version: \"%s\"\n", version);

    // Load opengl functions
    if (!loadAllFunctions()) {
        debugPrintf("Could not load all gl functions!\n");
        debugWaitForConsoleInput();
        return false;
    }

    // Enable debug output
    {
        glDebugMessageCallback(&customDebugProc, nullptr);
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    }

    ShowWindow(hwnd, SW_SHOWNORMAL);
    SetActiveWindow(hwnd);
    return true;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE unused, PSTR cmdLine, int cmdShow)
{
    globalInstance = instance;

    // Init subsystems
    memset(&gameState, 0, sizeof(GameState));
    CHECK_VALID(initDebugConsole(), "Console initialization failed\n");
    initKeyTranslationTable();
    initTiming();
    initFileListener("build\\game_tmp.dll");
    initDynamicLoading();
    initAllocators();
    //initAudio();

    // Initialize GameState
    {
        Memory* mem = &gameState.memory;
        mem->size = 1024L * 1024L * 4L;
        debugPrintf("Memory size: %lld\n", mem->size);
        mem->memory = (byte*) VirtualAlloc(NULL, mem->size, MEM_COMMIT, PAGE_READWRITE);
        if (mem->memory == NULL) {
            debugPrintf("VirtualAlloc failed: null returend\n");
            printLastError();
            return -1;
        }
    }

    // Init services
    {
        Services* services = &gameState.services;
        services->debugPrint = &debugPrint;
    }

    // Initialize window with OpenGL
    CHECK_VALID(initWindow(), "InitWindow failed\n");
    CHECK_VALID(createWindowWithOpenGL(), "Init opengl failed\n");
    CHECK_VALID(initRenderer(), "Error initializing renderer\n");

    // Initialize Game
    gameInit(&gameState);

    // Timing stuff
    double frameStart = currentTime();
    double tslf = 0;
    double gameTickTime = 0.0;
    double gameStartTime = frameStart;
    int& fps = actualWinState.fps;

    // MSG Loop
    MSG msg;
    bool quit = false;
    while (!quit) 
    {
        WindowState& desiredWinState = gameState.windowState;
        if (!actualWinState.continuousDraw) 
        {
            // Block until one message is received
            quit = (GetMessage(&msg, NULL, 0, 0) == 0);
            if (quit) { continue; }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Process all available messages
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0) 
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT) {
                quit = true;
            }
        }

        // Do timing
        gameState.time.now = frameStart - gameStartTime;
        gameState.time.tslf = tslf;
        gameState.time.lastGameTick = gameTickTime;

        // GameTick...
        debugGameTick();
        //gameTick(&gameState);
        resetInputState();
        //renderToWindow(deviceContext, &bitmapInfo, &gameState.videoData);

        // Handle requests
        if (desiredWinState.quit) {
            DestroyWindow(hwnd);
        }
        if (actualWinState.fullscreen != desiredWinState.fullscreen) 
        {
            if (desiredWinState.fullscreen) 
            {
                // Save current window pos
                GetWindowRect(hwnd, &savedWindowPos);

                // Switch to fullscreen 
                HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
                MONITORINFO info;
                info.cbSize = sizeof(MONITORINFO);
                if (GetMonitorInfo(monitor, &info) == 0) {
                    debugPrint("GetMonitorInfo failed\n");
                }
                else {
                    SetWindowLong(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
                    SetWindowLong(hwnd, GWL_EXSTYLE, NULL);
                    RECT& r = info.rcMonitor;
                    if (SetWindowPos(hwnd, HWND_TOP, r.left, r.top, r.right - r.left, r.bottom - r.top, NULL) != NULL)  {
                        actualWinState.fullscreen = true;
                    }
                }
            }
            else {
                SetWindowLong(hwnd, GWL_STYLE, windowStyle | WS_VISIBLE);
                SetWindowLong(hwnd, GWL_EXSTYLE, windowStyleEX);
                RECT& r = savedWindowPos;
                if (SetWindowPos(hwnd, HWND_TOP, r.left, r.top, r.right - r.left, r.bottom - r.top, NULL) != NULL)  {
                    actualWinState.fullscreen = false;
                }
            }
        }
        actualWinState.continuousDraw = desiredWinState.continuousDraw;
        if (desiredWinState.fps != fps) {
            if (desiredWinState.fps > 1) {
                fps = desiredWinState.fps;
            }
        }

        // Do timing stuff
        double gameTickEnd = currentTime();
        double frameTime = 1.0 / fps;
        gameTickTime = gameTickEnd - frameStart;
        // Sleep if possible
        sleepUntil(frameStart + frameTime);

        double frameEnd = currentTime();
        tslf = frameEnd - frameStart;
        frameStart = frameEnd;

        if (checkFileChanged()) {
            loadGameFunctions();
        }

        // Debug print
        /*{
            double sleepFor =  (frameStart + frameTime) - gameTickEnd;
            if (gameTickTime > frameTime) {
                debugPrintf("Frame took: %3.2fms, Tick: %3.2fms, OVER TIME\n", tslf * 1000, gameTickTime * 1000);
            }
            else {
                debugPrintf("Frame took: %3.2fms, Tick: %3.2fms\n", tslf * 1000, gameTickTime * 1000);
            }
        } */
    }

    gameShutdown(&gameState);

    debugPrint("Program exit\n");
    debugWaitForConsoleInput();

    return 0;
}
