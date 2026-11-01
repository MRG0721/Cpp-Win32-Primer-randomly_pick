// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <string>
#include <random>

#if defined(_MSC_VER)
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#endif

#define IDI_APPICON 101
#define IDC_EDIT_MIN       1001
#define IDC_EDIT_MAX       1002
#define IDC_BTN_GENERATE   1003
#define IDC_STATIC_RESULT  1004

static HWND g_hEditMin = nullptr;
static HWND g_hEditMax = nullptr;
static HWND g_hStaticResult = nullptr;

static std::mt19937 g_rng(std::random_device{}());

static std::wstring GetWindowTextString(HWND hWnd)
{
    int len = GetWindowTextLengthW(hWnd);
    std::wstring str(static_cast<size_t>(len) + 1, L'\0');
    GetWindowTextW(hWnd, &str[0], len + 1);
    str.resize(static_cast<size_t>(len));
    return str;
}

static bool ParseInt(const std::wstring& text, int& value)
{
    size_t first = text.find_first_not_of(L" \t\r\n");
    if (first == std::wstring::npos)
    {
        return false;
    }

    size_t last = text.find_last_not_of(L" \t\r\n");
    std::wstring trimmed = text.substr(first, last - first + 1);

    try
    {
        size_t pos = 0;
        int v = std::stoi(trimmed, &pos, 10);
        if (pos != trimmed.size())
        {
            return false;
        }
        value = v;
        return true;
    }
    catch (...)
    {
        return false;
    }
}

static BOOL CALLBACK SetFontProc(HWND hChild, LPARAM lParam)
{
    SendMessageW(hChild, WM_SETFONT, static_cast<WPARAM>(lParam), TRUE);
    return TRUE;
}

static void OnGenerate(HWND hWnd)
{
    int nMin = 0;
    int nMax = 0;

    if (!ParseInt(GetWindowTextString(g_hEditMin), nMin))
    {
        MessageBoxW(hWnd, L"请输入合法的最小值（整数）。", L"输入错误",
                    MB_OK | MB_ICONWARNING);
        SetFocus(g_hEditMin);
        return;
    }

    if (!ParseInt(GetWindowTextString(g_hEditMax), nMax))
    {
        MessageBoxW(hWnd, L"请输入合法的最大值（整数）。", L"输入错误",
                    MB_OK | MB_ICONWARNING);
        SetFocus(g_hEditMax);
        return;
    }

    if (nMin > nMax)
    {
        MessageBoxW(hWnd, L"最大值必须大于或等于最小值。", L"输入错误",
                    MB_OK | MB_ICONWARNING);
        SetFocus(g_hEditMax);
        return;
    }

    std::uniform_int_distribution<int> dist(nMin, nMax);
    int result = dist(g_rng);

    wchar_t buffer[64];
    wsprintfW(buffer, L"%d", result);
    SetWindowTextW(g_hStaticResult, buffer);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        HINSTANCE hInst = reinterpret_cast<LPCREATESTRUCTW>(lParam)->hInstance;

        CreateWindowExW(0, L"STATIC", L"最小标号：",
                        WS_CHILD | WS_VISIBLE | SS_RIGHT,
                        15, 28, 70, 22,
                        hWnd, nullptr, hInst, nullptr);

        g_hEditMin = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"1",
                                     WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                                     95, 25, 130, 26,
                                     hWnd, reinterpret_cast<HMENU>(IDC_EDIT_MIN),
                                     hInst, nullptr);

        CreateWindowExW(0, L"STATIC", L"最大标号：",
                        WS_CHILD | WS_VISIBLE | SS_RIGHT,
                        15, 68, 70, 22,
                        hWnd, nullptr, hInst, nullptr);

        g_hEditMax = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"100",
                                     WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                                     95, 65, 130, 26,
                                     hWnd, reinterpret_cast<HMENU>(IDC_EDIT_MAX),
                                     hInst, nullptr);

        CreateWindowExW(0, L"BUTTON", L"开抽",
                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                        15, 110, 210, 34,
                        hWnd, reinterpret_cast<HMENU>(IDC_BTN_GENERATE),
                        hInst, nullptr);

        g_hStaticResult = CreateWindowExW(WS_EX_CLIENTEDGE, L"STATIC", L"—",
                                          WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
                                          15, 160, 210, 44,
                                          hWnd, reinterpret_cast<HMENU>(IDC_STATIC_RESULT),
                                          hInst, nullptr);

        HFONT hFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        EnumChildWindows(hWnd, SetFontProc, reinterpret_cast<LPARAM>(hFont));

        return 0;
    }

    case WM_COMMAND:
    {
        if (LOWORD(wParam) == IDC_BTN_GENERATE && HIWORD(wParam) == BN_CLICKED)
        {
            OnGenerate(hWnd);
        }
        return 0;
    }

    case WM_CLOSE:
    {
        DestroyWindow(hWnd);
        return 0;
    }

    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }

    default:
        break;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PWSTR pCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(pCmdLine);

    const wchar_t* kClassName = L"RandomIntWindowClass";

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APPICON));    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = kClassName;
    wc.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APPICON));
    if (!RegisterClassExW(&wc))
    {
        MessageBoxW(nullptr, L"注册窗口类失败！", L"错误", MB_OK | MB_ICONERROR);
        return 0;
    }

    HWND hWnd = CreateWindowExW(0, kClassName, L"抽个群U",
                                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                CW_USEDEFAULT, CW_USEDEFAULT, 340, 270,
                                nullptr, nullptr, hInstance, nullptr);

    if (hWnd == nullptr)
    {
        MessageBoxW(nullptr, L"创建窗口失败！", L"错误", MB_OK | MB_ICONERROR);
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        if (!IsDialogMessageW(hWnd, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
}