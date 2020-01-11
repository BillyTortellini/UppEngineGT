#include <windows.h>
#include <cstring>

#include "..\datatypes.hpp"

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
            //PAINTSTRUCT ps;
            //HDC hdc = BeginPaint(hwnd, &ps);

            //// Paint
            //FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW));

            //EndPaint(hwnd, &ps);
            break;
    }

    return DefWindowProc(hwnd, msgType, wParam, lParam);
}

#define CHECK_VALID(cond, msg) if (!(cond)) {debugPrint(msg); return -1;}
int WINAPI WinMain(HINSTANCE instance, HINSTANCE unused, PSTR cmdLine, int cmdShow)
{
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
    }

    // Initialize Drawing with GDI
    {
        HDC deviceContext = GetDC(hwnd);
        CHECK_VALID(deviceContext != 0, "GetDC failed\n");

        RECT r;
        GetClientRect(hwnd, &r);
        int width = r.right;
        int height = r.bottom;

        int bytesPerPixel = 4;
        int imgSize = bytesPerPixel * width * height;
        byte* data = (byte*) VirtualAlloc(NULL, imgSize, MEM_COMMIT, PAGE_READWRITE);

        BITMAPINFO info = {};
        info.bmiHeader.biSize = sizeof(info);
        info.bmiHeader.biWidth = width;
        info.bmiHeader.biHeight = height;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;
        info.bmiHeader.biSizeImage = imgSize;
        info.bmiHeader.biXPelsPerMeter = 0;
        info.bmiHeader.biYPelsPerMeter = 0;
        info.bmiHeader.biClrUsed = 0;
        info.bmiHeader.biClrImportant = 0;

        memset(data, 0, imgSize);

        StretchDIBits (
                deviceContext,
                0, 0, width, height,
                0, 0, width, height,
                data, 
                &info,
                DIB_RGB_COLORS,
                SRCCOPY);
    }

    // MSG Loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) != NULL) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        debugPrint("GetMessage\n");
    }

    debugPrint("Program exit\n");
    debugWaitForConsoleInput();

    return 0;
}
