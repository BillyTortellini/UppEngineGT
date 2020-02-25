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
#include "../utils/tmpAlloc.hpp"
#include "win32_fileListener.cpp"

// ---------------
// --- GLOBALS ---
// ---------------
GameState gameState = {};

// WindowState needs to be global because it is used in message callback
struct ActualWinState
{
    int width;
    int height;
    int x;
    int y;
    bool fullscreen;
    bool minimized;
    bool hideCursor;
    bool inFocus;

    int fps;
    bool vsync;
    int continuousDraw;
    bool redraw;

    RECT savedWindowRect;
    LONG savedWindowStyle;
    LONG savedWindowStyleEx;
};
ActualWinState actualWinState;




// -----------------------
// --- DEBUG FUNCTIONS ---
// -----------------------
void win32_invalid_path(const char* str) 
{
    MessageBox(NULL, str, "INVALID_PATH", MB_OK);
    __debugbreak();
    exit(EXIT_FAILURE);
}

HANDLE consoleOutHandle;
HANDLE consoleInHandle;
void initDebugConsole() 
{
    BOOL success = AllocConsole();
    if (!success) {
        win32_invalid_path("AllocConsole failed\n");
    }

    consoleOutHandle= GetStdHandle(STD_OUTPUT_HANDLE);
    consoleInHandle = GetStdHandle(STD_INPUT_HANDLE);

    if (consoleOutHandle == INVALID_HANDLE_VALUE || 
            consoleInHandle == INVALID_HANDLE_VALUE) {
        win32_invalid_path("GetStdHandle failed\n");
    }
}

void debugPrint(const char* str) 
{
    DWORD out;
    WriteConsole(consoleOutHandle, str, (DWORD)strlen(str), &out, NULL);
}

void debugRead(char* buffer, uint32 size) 
{
    CONSOLE_READCONSOLE_CONTROL c;
    c.nLength = sizeof(c);
    c.nInitialChars = 0;
    c.dwCtrlWakeupMask = '\n';
    c.dwControlKeyState = 0;

    DWORD actuallyRead;
    ReadConsole(consoleInHandle, buffer, size, &actuallyRead, &c);
}

void debugWaitForConsoleInput() 
{
    char buf[256];
    debugRead(buf, sizeof(buf));
}

void printLastError() 
{
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



// ---------------------
// --- INPUT HANDING ---
// ---------------------
byte keyTranslationTable[NUM_KEYS];
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
    keyTranslationTable[VK_SHIFT] = KEY_SHIFT;
    keyTranslationTable[VK_CONTROL] = KEY_CTRL;
    //keyTranslationTable[] = KEY_LALT;
    keyTranslationTable[VK_RCONTROL] = KEY_RCTRL;
    keyTranslationTable[VK_RSHIFT] = KEY_RSHIFT;
    //keyTranslationTable[] = KEY_RALT;
}

// Gets called once after each gameTick
void resetInputState() 
{
    memset(&gameState.input.keyPressed, 0, NUM_KEYS);
    memset(&gameState.input.mousePressed, 0, NUM_MOUSE_KEYS);
    gameState.input.deltaX = 0;
    gameState.input.deltaY = 0;
    gameState.input.mouseWheel = 0;
    gameState.windowState.wasResized = false;
}

void initInput()
{
    resetInputState();
    initKeyTranslationTable();
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
        case WM_ACTIVATE:
            actualWinState.inFocus = !(LOWORD(wParam) == WA_INACTIVE);
            break;

        case WM_SIZE:
            {
                // Set new parameters
                if (wParam == SIZE_MINIMIZED) {
                    actualWinState.minimized = true;
                    gameState.windowState.minimized = true;
                    // Indicate change
                    gameState.windowState.wasMinimized = true;
                    return 0;
                }
                actualWinState.width = LOWORD(lParam); // Is in client area
                actualWinState.height = HIWORD(lParam); // Is in client area
                actualWinState.minimized = false;
                gameState.windowState.width = actualWinState.width;
                gameState.windowState.height = actualWinState.height;
                gameState.windowState.minimized = false;

                // Indicate change
                gameState.windowState.wasResized = true;

                return 0;
            }
        case WM_MOVE:
            actualWinState.x = (i16)LOWORD(lParam);
            actualWinState.y = (i16)HIWORD(lParam);
            gameState.windowState.x = actualWinState.x;
            gameState.windowState.y = actualWinState.y;

            // Indicate change
            if (!actualWinState.minimized) {
                gameState.windowState.wasMoved = true;
            }
            return 0;
    }

    return DefWindowProc(hwnd, msgType, wParam, lParam);
}



// ------------------------------
// --- DYNAMIC GAME RELOADING ---
// ------------------------------
typedef void (*gameInitFunc)(GameState* state); // Gets called once at program startup
typedef void (*gameTickFunc)(GameState* state); // Gets called every tick 
typedef void (*gameShutdownFunc)(GameState* state); // Gets called at program end
typedef void (*gameAudioFunc)(GameState* state, byte* stream, int length); // Gets called on each audio stream
typedef void (*gameBeforeResetFunc)(GameState* state); // Gets called before each reset
typedef void (*gameAfterResetFunc)(GameState* state); // Gets called after each reset
typedef void (*gameLoadFunctionPtrsFunc)(void** functions); // Gets called each time game loads

gameInitFunc gameInit;
gameTickFunc gameTick;
gameShutdownFunc gameShutdown;
gameAudioFunc gameAudio;
gameLoadFunctionPtrsFunc gameLoadFunctionPtrs;
gameBeforeResetFunc gameBeforeReset;
gameAfterResetFunc gameAfterReset;

void fallbackFunction(GameState* g) {}
void fallbackGameAudio(GameState* g, byte* stream, int length) {}
void fallbackGameLoadFunctionPtrs(void**) {}

void setToFallback()
{
    gameInit = &fallbackFunction;
    gameTick = &fallbackFunction;
    gameShutdown = &fallbackFunction;
    gameBeforeReset = &fallbackFunction;
    gameAfterReset = &fallbackFunction;
    gameAudio = &fallbackGameAudio;
    gameLoadFunctionPtrs = &fallbackGameLoadFunctionPtrs;
}

void setGameFunctionPtrs()
{
    void* functionPtrs[] = 
    {
        // Debug functions (For uppLib)
        &debugPrint,
        &win32_invalid_path,
        // FileListener functions
        &initFileListener,
        &checkFilesChanged,
        &createFileListener,
        &deleteFileListener,
        // OpenGL functions
        glDebugMessageCallback,
        glGenVertexArrays,
        glBindVertexArray,
        glGenBuffers,
        glBindBuffer,
        glBufferData,
        glVertexAttribPointer,
        glEnableVertexAttribArray,
        glUseProgram,
        glCreateShader,
        glShaderSource,
        glCompileShader,
        glDeleteShader,
        glCreateProgram,
        glDeleteProgram,
        glAttachShader,
        glDetachShader,
        glLinkProgram,
        glGetShaderiv,
        glGetShaderInfoLog,
        glGetProgramiv,
        glGetProgramInfoLog,
        glGetActiveUniform,
        glGetUniformLocation,
        glUniform1f,
        glUniform2f,
        glUniform3f,
        glUniform4f,
        glUniform1i,
        glUniform2i,
        glUniform3i,
        glUniform4i,
        glUniform1ui,
        glUniform2ui,
        glUniform3ui,
        glUniform4ui,
        glUniform1fv,
        glUniform2fv,
        glUniform3fv,
        glUniform4fv,
        glUniform1iv,
        glUniform2iv,
        glUniform3iv,
        glUniform4iv,
        glUniform1uiv,
        glUniform2uiv,
        glUniform3uiv,
        glUniform4uiv,
        glUniformMatrix2fv,
        glUniformMatrix3fv,
        glUniformMatrix4fv,
        glUniformMatrix2x3fv,
        glUniformMatrix3x2fv,
        glUniformMatrix2x4fv,
        glUniformMatrix4x2fv,
        glUniformMatrix3x4fv,
        glUniformMatrix4x3fv,
        glGetActiveAttrib,
        glGetAttribLocation,
        glDeleteBuffers,
        glDeleteVertexArrays,
        glGetStringi,
        wglSwapIntervalEXT,
        wglGetExtensionsStringARB,
        glGenerateMipmap,
        glActiveTexture,
        glGenFramebuffers,
        glBindFramebuffer,
        glDeleteFramebuffers,
        glCheckFramebufferStatus,
        glFramebufferTexture2D,
        glFramebufferRenderbuffer,
        glGenRenderbuffers,
        glDeleteRenderbuffers,
        glBindRenderbuffer,
        glRenderbufferStorage
    };

    gameLoadFunctionPtrs(functionPtrs);
}

HMODULE gameLibrary = NULL;
void loadGameFunctions()
{
    loggf("LOAD GAME FUNCTIONS\n");

    // Free current library so dll can be copied
    if (gameLibrary != NULL) {
        FreeLibrary(gameLibrary);
    }

    // Copy the file
    bool res = CopyFile("build\\game_tmp.dll", "build\\game_inUse.dll", FALSE);
    if (!res) {
        loggf("Could not copy file game_tmp.dll\n");
        setToFallback();
        return;
    }

    // Load library
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
    gameLoadFunctionPtrs = (gameLoadFunctionPtrsFunc) GetProcAddress(gameLibrary, "gameLoadFunctionPtrs");
    gameBeforeReset = (gameBeforeResetFunc) GetProcAddress(gameLibrary, "gameBeforeReset");
    gameAfterReset = (gameAfterResetFunc) GetProcAddress(gameLibrary, "gameAfterReset");
    if (gameInit == NULL || gameTick == NULL || gameShutdown == NULL 
            || gameAudio == NULL || gameLoadFunctionPtrs == NULL || gameBeforeReset == NULL || gameAfterReset == NULL)
    {
        loggf("GetProcAddress failed: One or more functions were not found in dll.\n");
        setToFallback();
        return;
    }
}

bool changedPrevFrame = false;
bool changedThisFrame = false;
void onGameDllChanged(const char* filename, void* userData) 
{
    if (!changedPrevFrame && !changedThisFrame) 
    {
        gameBeforeReset(&gameState);
        printFileListeners();
        loadGameFunctions();
        setGameFunctionPtrs();
        gameAfterReset(&gameState);
        changedThisFrame = true;
    }
    changedPrevFrame = changedThisFrame;
}

void initDynamicReloading()
{
    ListenerToken dllListener = 
        createFileListener("build\\game_tmp.dll", &onGameDllChanged, nullptr);
    loadGameFunctions();
    setGameFunctionPtrs();
}



// --------------
// --- TIMING ---
// --------------
int64 performanceFrequency;
void initTiming() 
{
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

    // To make sure that time is as accurate as possible, 
    // we sleep one ms less and do busy waiting
    ms -= 1;

    if (ms > 0) {
        timeBeginPeriod(1);
        Sleep(ms);
        timeEndPeriod(1);
    }

    // Busy wait until time actually passed
    do {} while (currentTime() < until);
}

void sleepFor(double seconds) 
{
    double start = currentTime();
    sleepUntil(start + seconds);
}


// -------------
// --- AUDIO ---
// -------------
void initAudio()
{
    HMODULE directSoundLib = LoadLibrary("dsound.dll");
    if (directSoundLib)
    {
    }
    invalid_path("Audio not implemented yet\n");
}



// -----------------------
// --- WINDOW CREATION ---
// -----------------------
void createWindowClass(HINSTANCE instance, const char* className)
{
    // Create window class
    WNDCLASS wc = {};
    wc.style = CS_HREDRAW  | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = &windowProc;
    wc.cbClsExtra = 0;
    wc.hInstance = instance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = className;

    if (RegisterClass(&wc) == 0) {
        invalid_path("RegisterClass failed!\n");
    }
}

HWND createWindow(HINSTANCE instance, bool show, 
        const char* className, const char* windowName)
{
    int windowStyle = WS_OVERLAPPEDWINDOW;
    int windowStyleEX = WS_EX_OVERLAPPEDWINDOW;

    // Create window
    HWND hwnd = CreateWindowEx(
            windowStyleEX,
            className,
            windowName,
            windowStyle,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            NULL,
            NULL,
            instance,
            NULL);

    assert(hwnd != NULL, "CreateWindowEX failed\n");

    // Show window if specified
    if (show) {
        ShowWindow(hwnd, SW_SHOW);
    }

    return hwnd;
}

HGLRC createDummyContext(HDC deviceContext)
{
    // Set pixelFormatDescriptor
    PIXELFORMATDESCRIPTOR pfd;
    {
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
    }

    // Choose pixel format only returns an approximate format
    int index = ChoosePixelFormat(deviceContext, &pfd);
    assert(index != NULL, "ChoosePixelFormat failed\n");

    // Check if the closest pixel format supports what we need
    {
        PIXELFORMATDESCRIPTOR available;
        int ret = DescribePixelFormat(deviceContext, index, 
                sizeof(PIXELFORMATDESCRIPTOR), &available);
        assert(ret != NULL, "DescribePixelFormat failed\n");

        // Following attributes are required:
        // PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER
        // Also pixelType must match (RGBA);
        bool success = 
            (available.dwFlags | PFD_DRAW_TO_WINDOW) &&
            (available.dwFlags | PFD_SUPPORT_OPENGL) &&
            (available.dwFlags | PFD_DOUBLEBUFFER) &&
            (available.iPixelType == pfd.iPixelType);
        assert(success, "Could not get pixelFormat that matches required format\n");
    }

    // Set pixel format
    bool success = SetPixelFormat(deviceContext, index, &pfd);
    assert(success, "SetPixelFormat failed\n");

    HGLRC glContext = wglCreateContext(deviceContext);
    assert(glContext != NULL, "wglCreateContext failed\n");

    return glContext;
}

void customDebugProc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, 
        const GLchar* message, const void* userParam)
{
    // Ignore unecessary warniings
    // 131186 Performace warnings with static_draw using video memory
    // 131218 Performance wanring vertex shader ins being recompiled (TODO: Check what this means)
    if (id == 0 || id == 131185 || id == 131218) return;

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

void initWindowAndOpenGL(HINSTANCE instance, HWND& hwnd, HDC& deviceContext, HGLRC& glContext)
{
    const char* className = "UppEngineGT";
    const char* windowName = "UppEngineGT";
    createWindowClass(instance, className);

    // Create dummy window for dummy context
    hwnd = createWindow(instance, false, className, windowName);

    // Get window device context
    deviceContext = GetDC(hwnd);
    assert(deviceContext != 0, "GetDC failed\n");

    // Create dummy context
    glContext = createDummyContext(deviceContext);
    BOOL success = wglMakeCurrent(deviceContext, glContext);
    assert(glContext != NULL, "wglMakeCurrent failed\n");

    // DEBUG START
    loggf("Dummy context creation worked!\n");
    char* version = (char*)glGetString(GL_VERSION);
    loggf("dummy context version: \"%s\"\n", version);
    // DEBUG END

    // Get createContextAttribs function
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) 
        getAnyGLFuncAddress("wglCreateContextAttribsARB");
    assert(wglCreateContextAttribsARB != NULL, 
            "Get any function failed for createContextAttribs\n");

    // Destroy dummy context
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(glContext);

    // TODO
    // Destroy current hwnd and do pixelformat stuff again
    // with the better functions from the real context

    // Create real context
    int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4, // Opengl Version
        WGL_CONTEXT_MINOR_VERSION_ARB, 5,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB, // Core Profile
        0 // MUST STAY, this is the end marker for wlgCreateContextAtribsARB
    };

    glContext = wglCreateContextAttribsARB(deviceContext, NULL, attribs);
    assert(glContext != NULL, "wglCreateContextAttribsARB failed\n");

    // Make context current
    success = wglMakeCurrent(deviceContext, glContext);
    assert(success, "wglMakeCurrent failed!\n");

    loggf("Initializisation version 4.5 worked!!!\n");
    version = (char*)glGetString(GL_VERSION);
    loggf("version: \"%s\"\n", version);

    // Load opengl functions
    success = loadAllFunctions();
    assert(success, "loadAllFunctions failed!\n");

    // Enable debug output
    glDebugMessageCallback(&customDebugProc, nullptr);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    // Set vsync to on
    wglSwapIntervalEXT(1);

    // Show window again
    ShowWindow(hwnd, SW_SHOWNORMAL);
    SetActiveWindow(hwnd);
}



// -----------------------
// --- STATE FUNCTIONS ---
// -----------------------
void retrieveWindowSize(HWND hwnd) 
{
    RECT clientRect;
    GetWindowRect(hwnd, &actualWinState.savedWindowRect);
    GetClientRect(hwnd, &clientRect);

    actualWinState.x = actualWinState.savedWindowRect.left;
    actualWinState.y = actualWinState.savedWindowRect.top;
    actualWinState.width = clientRect.right - clientRect.left;
    actualWinState.height = clientRect.bottom - clientRect.top;
    WindowState* w = &gameState.windowState;
    ActualWinState* a = &actualWinState;
    if (w->width != a->width || w->height != a->height) {
        a->width = w->width;
        a->height = w->height;
        w->wasResized = true;
    }
    if (w->x != a->x || w->y != a->y) {
        a->x = w->x;
        a->y = w->y;
        w->wasMoved = true;
    }
}

void initWindowState(HWND hwnd)
{
    // Init actual window state
    retrieveWindowSize(hwnd);
    actualWinState.fullscreen = false;
    actualWinState.minimized = false;
    actualWinState.continuousDraw = true;
    actualWinState.redraw = false;
    actualWinState.fps = 60;
    actualWinState.hideCursor = false;
    actualWinState.inFocus = true;

    // Save window style
    actualWinState.savedWindowStyle = GetWindowLong(hwnd, GWL_STYLE);
    actualWinState.savedWindowStyleEx = GetWindowLong(hwnd, GWL_EXSTYLE);
}

void initGameState()
{
    // Set windowState
    WindowState* w = &gameState.windowState;
    w->wasResized = false;
    w->wasMoved = false;
    w->wasMinimized = false;
    w->quit = false;
    w->fullscreen = actualWinState.fullscreen;
    w->hideCursor = actualWinState.hideCursor;
    w->minimized = actualWinState.minimized;
    w->width = actualWinState.width;
    w->height = actualWinState.height;
    w->x = actualWinState.x;
    w->y = actualWinState.y;
    w->inFocus = actualWinState.inFocus;

    // Set input
    memset(&gameState.input, 0, sizeof(Input));

    // Set RenderOptions
    RenderOptions* r = &gameState.renderOptions;
    r->vsync = true;
    r->continuousDraw = true;
    r->redraw = false;
    r->fps = 60;

    // Set SoundInfo
    memset(&gameState.soundInfo, 0, sizeof(SoundInfo));

    // Set Memory
    gameState.memory.size = 1024L * 1024L * 1024L * 1L; // 1 GB
    gameState.memory.data = VirtualAlloc(NULL, gameState.memory.size, 
            MEM_COMMIT, PAGE_READWRITE);
    assert(gameState.memory.data != NULL, "Virtual Alloc failed\n");

    // Set Time
    gameState.time.now = currentTime();
    gameState.time.tslf = 0;
}



// --------------------------
// --- MSG LOOP FUNCTIONS ---
// --------------------------
bool processWindowMessages()
{
    bool quit = false;
    MSG msg;
    if (!actualWinState.continuousDraw) 
    {
        // Block until one message is received
        quit = (GetMessage(&msg, NULL, 0, 0) == 0);
        if (quit) { return false; }
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

    return quit;
}

bool handleGameRequests(HWND hwnd)
{
    // Helpers
    WindowState* w = &gameState.windowState;
    RenderOptions* o = &gameState.renderOptions;
    ActualWinState* actual = &actualWinState;

    // Simple updates
    actual->redraw = o->redraw;
    actual->continuousDraw = o->continuousDraw;
    actual->fps = o->fps;
    w->inFocus = actual->inFocus;

    // Update confinement if window was moved or resized
    if (actual->hideCursor && (w->wasResized || w->wasMoved)) {
        RECT clip;
        GetWindowRect(hwnd, &clip);
        loggf("Updating cursor clip\n");
        ClipCursor(&clip);
    }

    // Reset members that get reset each frame
    w->wasResized = false;
    w->wasMoved = false;
    w->wasMinimized = false;

    // Handle quit request
    if (w->quit) {
        return true;
    }

    // Handle minimize request
    if (w->minimized != actual->minimized) 
    {
        actual->minimized = w->minimized;
        if (w->minimized) {
            ShowWindow(hwnd, SW_MINIMIZE);
        }
        else {
            ShowWindow(hwnd, SW_RESTORE);
        }
    }

    // Handle vsynch request
    if (o->vsync != actual->vsync) {
        actual->vsync = o->vsync;
        if (o->vsync) {
            loggf("Vsync set to on\n");
            wglSwapIntervalEXT(1);
        }
        else {
            loggf("Vsync set to off\n");
            wglSwapIntervalEXT(0);
        }
    }

    // Handle request window move/resize
    if (actual->x != w->x || actual->y != w->y ||
            actual->width != w->width || actual->height != w->height) {
        loggf("Sizes did not match\n");
        // Ignore if fullscreen is set or if minimized
        if (!actual->fullscreen) 
        {
            SetWindowPos(hwnd, HWND_TOP, w->x, w->y, w->width, w->height, NULL);
            w->wasMoved = true;
            if (actual->x != w->x || actual->y != w->y) {
                w->wasMoved = true;
            }
            else {
                w->wasResized = true;
            }
            retrieveWindowSize(hwnd);
        }
        else {
            w->x = actual->x;
            w->y = actual->y;
            w->width = actual->width;
            w->height = actual->height;
            w->wasResized = true;
            w->wasMoved = true;
        }
    }

    // Handle fullscreen requests
    if (actual->fullscreen != w->fullscreen) {
        actual->fullscreen = w->fullscreen;
        loggf("Window fullscreen request\n");
        if (w->fullscreen) 
        {
            // Save current window position and size
            GetWindowRect(hwnd, &actual->savedWindowRect);
            // Save window style
            actual->savedWindowStyle = GetWindowLong(hwnd, GWL_STYLE);
            actual->savedWindowStyleEx = GetWindowLong(hwnd, GWL_EXSTYLE);

            // Switch to fullscreen 
            HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
            MONITORINFO info;
            info.cbSize = sizeof(MONITORINFO);
            if (GetMonitorInfo(monitor, &info) == 0) {
                loggf("GetMonitorInfo failed, could not switch to fullscreen\n");
            }
            else 
            {
                // Enable fullscreen
                SetWindowLong(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
                SetWindowLong(hwnd, GWL_EXSTYLE, NULL);
                RECT& r = info.rcMonitor;
                SetWindowPos(hwnd, HWND_TOP, 
                        r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_DRAWFRAME | SWP_SHOWWINDOW);
                w->wasResized = true;
            }
        }
        else 
        {
            SetWindowLong(hwnd, GWL_STYLE, actual->savedWindowStyle | WS_VISIBLE);
            SetWindowLong(hwnd, GWL_EXSTYLE, actual->savedWindowStyleEx);
            RECT& r = actual->savedWindowRect;
            SetWindowPos(hwnd, HWND_TOP, r.left, r.top, 
                    r.right - r.left, r.bottom - r.top, NULL);
        }

        retrieveWindowSize(hwnd);
    }

    if (actual->hideCursor != w->hideCursor) 
    {
        actual->hideCursor = w->hideCursor;
        if (actual->hideCursor) 
        {
            ShowCursor(FALSE);
            // Confine cursor
            RECT clip;
            GetWindowRect(hwnd, &clip);
            ClipCursor(&clip);
        }
        else 
        {
            ShowCursor(TRUE);
            ClipCursor(NULL);
        }
    }

    return false;
}

void afterProcessWindowMessages(HWND hwnd) 
{
    // Reset cursor position if hideCursor is enable
    if (actualWinState.hideCursor) 
    {
        // Calculate middle of window pos
        POINT p;
        p.x = gameState.windowState.x + gameState.windowState.width/2;
        p.y = gameState.windowState.y + gameState.windowState.height/2;

        // Set to middle of window
        POINT oldPos;
        GetCursorPos(&oldPos);
        gameState.input.deltaX = oldPos.x - p.x; 
        gameState.input.deltaY = oldPos.y - p.y;
        SetCursorPos(p.x, p.y);
    }
}



// -----------------------
// --- DEBUG FUNCTIONS ---
// -----------------------
void debugGameTick() 
{
    Input& input = gameState.input;
    static bool first = true;
    if (first) 
    {
        gameState.renderOptions.continuousDraw = true;
        gameState.renderOptions.fps = 60;
        gameState.windowState.x = -1000;
        gameState.windowState.y = 400;
        gameState.windowState.fullscreen = true;
        first = false;
    }
    if (gameState.windowState.wasResized) {
        loggf("GAMETICK: resized to %d/%d\n", gameState.windowState.width, gameState.windowState.height);
    }
    if (gameState.windowState.wasMoved) {
        loggf("GAMETICK: moved to %d/%d\n", gameState.windowState.x, gameState.windowState.y);
    }
    if (gameState.windowState.wasMinimized) {
        loggf("GAMETICK: moved to %d/%d\n", gameState.windowState.x, gameState.windowState.y);
    }
    if (input.mouseWheel != 0) {
        loggf("MouseWheel: %f\n", input.mouseWheel);
    }
    if (input.keyPressed[KEY_F11]) {
        bool& f = gameState.windowState.fullscreen;
        f = !f;
        loggf("Fullscreen toggled to %s\n", f == true ? "TRUE" : "FALSE");
    }
    if (input.keyPressed[KEY_F5]) {
        //gameState.windowState.hideCursor = !gameState.windowState.hideCursor;
        gameState.windowState.minimized = !gameState.windowState.minimized;
        //gameState.windowState.x += 20;
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
}



// ------------
// --- MAIN ---
// ------------
int WINAPI WinMain(HINSTANCE instance, HINSTANCE unused, PSTR cmdLine, int cmdShow)
{
    // Init Debugging
    initDebugConsole(); 
    setDebugFunctions(&debugPrint, &win32_invalid_path);

    // Init TmpAlloc
    SystemAllocator sysAlloc;
    initTmpAlloc(&sysAlloc);
    SCOPE_EXIT(shutdownTmpAlloc());

    // Init window
    HWND hwnd;
    HDC deviceContext;
    HGLRC glContext;
    initWindowAndOpenGL(instance, hwnd, deviceContext, glContext);

    // Init subsystems
    initFileListener(&sysAlloc);
    initInput();
    initTiming();
    initDynamicReloading();
    initWindowState(hwnd);
    initGameState();

    logg("\n");
    logg("---------------------\n");
    logg("--- PROGRAM START ---\n");
    logg("---------------------\n");
    logg("\n");
    gameInit(&gameState);

    // Timing stuff
    double frameStart = currentTime();
    double gameStartTime = frameStart;

    // MSG Loop
    while (true) 
    {
        // Process messages and handle requests
        bool quit = false;
        quit |= handleGameRequests(hwnd); // This order is important!
        quit |= processWindowMessages();
        afterProcessWindowMessages(hwnd);
        if (quit) break;

        // Check if any files have changed
        changedThisFrame = false;
        checkFilesChanged();

        // Update game
        //debugGameTick();
        gameTick(&gameState);
        resetInputState();

        // Show buffer (Hint: Maybe wait for vblanc to swap?)
        if (actualWinState.continuousDraw || actualWinState.redraw) {
            if (actualWinState.vsync) {
                glFinish(); 
            }
            SwapBuffers(deviceContext);
            //if (actualWinState.vsync) {
            //    glFinish(); 
            //}
            actualWinState.redraw = false;
        }

        // Sleep until frame end
        double frameTime = 1.0 / actualWinState.fps;
        sleepUntil(frameStart + frameTime);

        // Calculate frame timing
        double frameEnd = currentTime();
        double tslf = frameEnd - frameStart;
        frameStart = frameEnd;

        // Set game time
        gameState.time.now = frameStart - gameStartTime; 
        gameState.time.tslf = tslf;
    }

    // Shutdown
    DestroyWindow(hwnd);
    gameShutdown(&gameState);
    loggf("Program exit\n");
    //sleepFor(2);
    //debugWaitForConsoleInput();

    return EXIT_SUCCESS;
}
