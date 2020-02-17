#include "platformSpecific.cpp"
#include "..\platform.hpp"

#include <uppLib.hpp>
#include <windows.h>
#include <windowsx.h>
#include <wingdi.h>
#include <cstring>
#include <cstdarg>
#include <initializer_list>
#include <intrin.h>
#include <GL/GL.h>
#include <GL/glext.h>
#include <GL/wglext.h>

// Undefine near and far macros that are defined in the windows headers
#undef near
#undef far

#include "win32_glFunctions.hpp"
#include "../tmpAlloc.hpp"
#include "../renderer.hpp"
#include "win32_fileListener.cpp"

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

void win32_invalid_path(const char* str) {
    MessageBox(NULL, str, "INVALID_PATH", MB_OK);
    __debugbreak();
    exit(EXIT_FAILURE);
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
    loggf("Last error code: %d, Error:\n%s\n", error, msgBuffer);
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


void resizeVideoBuffer() 
{
    loggf("ResizeVideoBuffer w/h: %d/%d\n", actualWinState.width, actualWinState.height); 
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
    loggf("LOAD GAME FUNCTIONS\n");
    if (gameLibrary != NULL) {
        FreeLibrary(gameLibrary);
    }

    bool res = CopyFile("build\\game_tmp.dll", "build\\game_inUse.dll", FALSE);
    if (!res) {
        loggf("Could not copy file game_tmp.dll\n");
        setToFallback();
        return;
    }

    gameLibrary = LoadLibrary("build\\game_inUse.dll");
    if (gameLibrary == NULL) {
        loggf("LoadGameFunctions failed: loadlibrary failed\n");
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
        loggf("GetProcAddress failed: One or more functions were not found in dll.\n");
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
    loggf("asdf audio\n");
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
        //loggf("Mouse X/Y: %d/%d, delta: %d/%d \n", input.mouseX, input.mouseY, input.deltaX, input.deltaY);
    }
    if (gameState.windowState.wasResized) {
        loggf("GAMETICK: resized to %d/%d\n", gameState.windowState.width, gameState.windowState.height);
    }
    if (input.mouseWheel != 0) {
        loggf("MouseWheel: %f\n", input.mouseWheel);
    }
    if (input.keyPressed[KEY_F5]) {
        bool& d = gameState.windowState.continuousDraw;
        d = !d;
        loggf("Continuous draw switched to %s\n", d == true ? "TRUE" : "FALSE");
    }
    if (input.keyPressed[KEY_F11]) {
        bool& f = gameState.windowState.fullscreen;
        f = !f;
        loggf("Fullscreen toggled to %s\n", f == true ? "TRUE" : "FALSE");
    }
    if (gameState.input.mousePressed[MOUSE_LEFT]) {
        debugPrint("Mouse Left pressed\n");
        loggf("MB_RIGHT: %d\n", MOUSE_RIGHT);
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
        loggf("ChoosePixelFormat failed\n");
        printLastError();
        return false;
    }

    // Check if the closest pixel format supports what we need
    PIXELFORMATDESCRIPTOR available;
    int ret = DescribePixelFormat(deviceContext, index, sizeof(PIXELFORMATDESCRIPTOR), &available);
    if (ret == NULL) {
        loggf("DescribePixelFormat failed\n");
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
        loggf("Available pixelformat does not fullfill the games requirements\n");
        return false;
    }

    // Set pixel format
    if (SetPixelFormat(deviceContext, index, &pfd) == FALSE) {
        loggf("SetPixelFormat failed\n");
        printLastError();
        return false;
    }

    // Create the OpenGL context
    glContext = wglCreateContext(deviceContext);
    if (glContext == NULL) {
        loggf("wglCreateContext failed\n");
        printLastError();
        return false;
    }

    if (wglMakeCurrent(deviceContext, glContext) == FALSE) {
        loggf("wglMakeCurrent failed\n");
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
    // 131186 Performace warnings with static_draw using video memory
    if (id == 0 || id == 131185) return;

    loggf("CustomDebugProc: ERROR ID: %d:, msg: \"%s\"\n", id, message);
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             loggf("Source: API\t"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   loggf("Source: Window System\t"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: loggf("Source: Shader Compiler\t"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     loggf("Source: Third Party\t"); break;
        case GL_DEBUG_SOURCE_APPLICATION:     loggf("Source: Application\t"); break;
        case GL_DEBUG_SOURCE_OTHER:           loggf("Source: Other\t"); break;
    } 

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               loggf("Type: Error\t"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: loggf("Type: Deprecated Behaviour\t"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  loggf("Type: Undefined Behaviour\t"); break; 
        case GL_DEBUG_TYPE_PORTABILITY:         loggf("Type: Portability\t"); break;
        case GL_DEBUG_TYPE_PERFORMANCE:         loggf("Type: Performance\t"); break;
        case GL_DEBUG_TYPE_MARKER:              loggf("Type: Marker\t"); break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          loggf("Type: Push Group\t"); break;
        case GL_DEBUG_TYPE_POP_GROUP:           loggf("Type: Pop Group\t"); break;
        case GL_DEBUG_TYPE_OTHER:               loggf("Type: Other\t"); break;
    } 

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         loggf("Severity: high\t"); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       loggf("Severity: medium\t"); break;
        case GL_DEBUG_SEVERITY_LOW:          loggf("Severity: low\t"); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: loggf("Severity: notification\t"); break;
    }
    loggf("\n");
}

PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
bool createWindowWithOpenGL()
{
    loggf("Init window with OpenGL\n");

    if (!createWindow(false)) {
        loggf("CreateWindow dummy failed\n");
        return false;
    }

    if (!createDummyContext()) {
        loggf("Create dummy context failed\n");
        return false;
    }

    loggf("Dummy context creation worked!\n");
    char* version = (char*)glGetString(GL_VERSION);
    loggf("dummy context version: \"%s\"\n", version);

    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) getAnyGLFuncAddress("wglCreateContextAttribsARB");
    if (wglCreateContextAttribsARB == NULL) {
        loggf("Get andy func failed for createContextAttribs\n");
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
        loggf("glContext was null!\n");
        debugWaitForConsoleInput();
        return false;
    }

    // Make context current
    if (wglMakeCurrent(deviceContext, glContext) == FALSE) {
        loggf("wglMakeCurrent failed\n");
        printLastError();
        return false;
    }

    loggf("Initializisation version 4.5 worked!!!\n");
    version = (char*)glGetString(GL_VERSION);
    loggf("version: \"%s\"\n", version);

    // Load opengl functions
    if (!loadAllFunctions()) {
        loggf("Could not load all gl functions!\n");
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

void onGameDllChanged(const char* filename, void* userData) {
    loadGameFunctions();
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE unused, PSTR cmdLine, int cmdShow)
{
    globalInstance = instance;

    // Init allocators
    SystemAllocator sysAlloc;
    initTmpAlloc(&sysAlloc);
    SCOPE_EXIT(shutdownTmpAlloc());

    // Init subsystems
    memset(&gameState, 0, sizeof(GameState));
    assert(initDebugConsole(), "Console initialization failed\n");
    setDebugFunctions(&debugPrint, &win32_invalid_path);
    initKeyTranslationTable();
    initTiming();
    initFileListener(&sysAlloc);
    //initAudio();

    // Init dynamic loading
    ListenerToken dllListener = createFileListener("build\\game_tmp.dll", &onGameDllChanged, nullptr);
    initDynamicLoading();

    // Initialize GameState
    {
        Memory* mem = &gameState.memory; mem->size = 1024L * 1024L * 4L;
        loggf("Memory size: %lld\n", mem->size);
        mem->memory = (byte*) VirtualAlloc(NULL, mem->size, MEM_COMMIT, PAGE_READWRITE);
        if (mem->memory == NULL) {
            loggf("VirtualAlloc failed: null returend\n");
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
    logg("\n");
    logg("---------------------\n");
    logg("--- PROGRAM START ---\n");
    logg("---------------------\n");
    CHECK_VALID(initRenderer(&sysAlloc), "Error initializing renderer\n");

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

        // Check if any files have changed
        checkFilesChanged();

        // Debug print
        /*{
            double sleepFor =  (frameStart + frameTime) - gameTickEnd;
            if (gameTickTime > frameTime) {
                loggf("Frame took: %3.2fms, Tick: %3.2fms, OVER TIME\n", tslf * 1000, gameTickTime * 1000);
            }
            else {
                loggf("Frame took: %3.2fms, Tick: %3.2fms\n", tslf * 1000, gameTickTime * 1000);
            }
        } */
    }

    gameShutdown(&gameState);

    debugPrint("Program exit\n");
    debugWaitForConsoleInput();

    return 0;
}
