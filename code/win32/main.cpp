#include <windows.h>
#include <cstring>

HANDLE consoleOutHandle;
void debugPrint(const char* str) {
    DWORD out;
    WriteConsole(consoleOutHandle, str, strlen(str), &out, NULL);
}

LRESULT windowProc(HWND hwnd, UINT msgType, WPARAM wParam, LPARAM lParam)
{
    switch (msgType)
    {
        case WM_CLOSE:
            debugPrint("WM_CLOSE\n");
            break;

        case WM_QUIT:
            debugPrint("WM_QUIT\n");
            break;

        case WM_DESTROY:
            debugPrint("WM_DESTORY\n");
            PostQuitMessage(0);
            break;
    }

    return DefWindowProc(hwnd, msgType, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE instance, HINSTANCE unused, PSTR cmdLine, int cmdShow)
{
    // Initialize Debug Console
    {
        if (AllocConsole() == NULL) {
            return -1;
        }
        consoleOutHandle= GetStdHandle(STD_OUTPUT_HANDLE);
        if (consoleOutHandle == INVALID_HANDLE_VALUE) {
            return -1;
        }
        debugPrint("Hello there");
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

    // MSG Loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) != NULL) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        debugPrint("GetMessage\n");
    }

    debugPrint("Program exit\n");

    return 0;
}
