#include <Windows.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <fstream>

#include "graphicsSystem.h"

#define WINDOW_NAME L"CS300! ^.^"

//Globals
namespace WinData
{
    HINSTANCE *hInst;
    WNDCLASS wndClass;
    HWND windowHandle;
    int windowHeight, windowWidth;
}

//Forward declarations
LRESULT CALLBACK WindowProc(_In_ HWND   hwnd, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

//Used to create a console window
void RedirectIOToConsole(FILE *consoleFP)
{
    AllocConsole();
    freopen_s(&consoleFP, "CONOUT$", "wb", stdout);
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
    //Initialize console
    FILE *consoleFP = 0;
    RedirectIOToConsole(consoleFP);

    //Set up global variables
    WinData::hInst = &hInstance;

    if (hInstance == NULL)
        hInstance = (HINSTANCE)GetModuleHandle(NULL);

    HICON hIcon = NULL;
    WCHAR szExePath[MAX_PATH];
    GetModuleFileName(NULL, szExePath, MAX_PATH);

    // If the icon is NULL, then use the first one found in the exe
    if (hIcon == NULL)
        hIcon = ExtractIcon(hInstance, szExePath, 0);

    using namespace WinData;
    // Set up WNDCLASS
    wndClass.style = CS_DBLCLKS;
    wndClass.lpfnWndProc = WindowProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hInstance;
    wndClass.hIcon = hIcon;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = L"CS300Proj";

    //Register the WNDCLASS
    if (!RegisterClass(&wndClass))
    {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_CLASS_ALREADY_EXISTS)
            return HRESULT_FROM_WIN32(dwError);
    }

    //Create the window
    int x = CW_USEDEFAULT;
    int y = CW_USEDEFAULT;

    // Resizable 640x480 window
    int wDefaultWidth = WinData::windowWidth = 640;
    int wDefaultHeight = WinData::windowHeight = 480;
    
    RECT windowRectangle;

    SetRect(&windowRectangle, 0, 0, wDefaultWidth, wDefaultHeight);
    AdjustWindowRect(&windowRectangle, WS_OVERLAPPEDWINDOW, false);

    // Create the window for our viewport.
    windowHandle = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, wndClass.lpszClassName, WINDOW_NAME, WS_OVERLAPPEDWINDOW, x, y, (windowRectangle.right - windowRectangle.left), (windowRectangle.bottom - windowRectangle.top),
                    0, NULL, hInstance, 0);


    if (windowHandle == NULL)
    {
        DWORD dwError = GetLastError();
        return HRESULT_FROM_WIN32(dwError);
    }

    ShowWindow(windowHandle, true);

    graphicsMainLoop();

    //Cleanup

    return 0;
}

//WinProc function for dealing with windows messages
LRESULT CALLBACK WindowProc(_In_ HWND   hwnd, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SIZE:
    {
        std::cout << "Window resized!\n";
        WinData::windowWidth = LOWORD(lParam);
        WinData::windowHeight = HIWORD(lParam);
        return 0;
    }

    case WM_CLOSE:
    {
        std::cout << "Requested to close window\n";
        HMENU hMenu;
        hMenu = GetMenu(hwnd);
        if (hMenu != NULL)
        {
            DestroyMenu(hMenu);
        }
        DestroyWindow(hwnd);
        UnregisterClass(
            L"CS300Proj",
            *WinData::hInst
        );
        return 0;
    }

    case WM_DESTROY:
        std::cout << "Window getting obliterated\n";
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

