#include <windows.h>
#include <commctrl.h>
#include <gdiplus.h>
#include <string>
#include <thread>
#include <TlHelp32.h>
#include <psapi.h>
#include <shlwapi.h>
#include <shellapi.h>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "shell32.lib")

using namespace Gdiplus;

const int WINDOW_WIDTH = 400;
const int WINDOW_HEIGHT = 500;
const wchar_t WINDOW_CLASS_NAME[] = L"AkuBotLauncherClass";
const wchar_t WINDOW_TITLE[] = L"AkuBot";

const COLORREF BACKGROUND_COLOR = RGB(30, 30, 40);
const COLORREF BUTTON_COLOR = RGB(102, 204, 255);
const COLORREF BUTTON_HOVER_COLOR = RGB(135, 220, 255);

HWND g_hwnd;
bool g_buttonHover = false;
HINSTANCE g_hInstance;

bool IsAimlabInstalled() {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (Process32First(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, L"Aimlab_tb.exe") == 0) {
                CloseHandle(snapshot);
                return true;
            }
        } while (Process32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);

    wchar_t drives[256];
    DWORD driveLength = GetLogicalDriveStrings(255, drives);

    if (driveLength > 0) {
        wchar_t* drive = drives;
        while (*drive) {
            UINT driveType = GetDriveType(drive);
            if (driveType == DRIVE_FIXED) {
                std::wstring basePath = drive;

                std::wstring paths[] = {
                    basePath + L"Program Files\\Aimlab\\Aimlab_tb.exe",
                    basePath + L"Program Files (x86)\\Aimlab\\Aimlab_tb.exe",
                    basePath + L"Program Files\\Steam\\steamapps\\common\\Aim Lab\\Aimlab_tb.exe",
                    basePath + L"Steam\\steamapps\\common\\Aim Lab\\Aimlab_tb.exe",
                    basePath + L"Games\\Steam\\steamapps\\common\\Aim Lab\\Aimlab_tb.exe",
                    basePath + L"SteamLibrary\\steamapps\\common\\Aim Lab\\Aimlab_tb.exe"
                };

                for (const std::wstring& path : paths) {
                    if (PathFileExists(path.c_str())) {
                        return true;
                    }
                }
            }
            drive += wcslen(drive) + 1;
        }
    }

    return false;
}

void RunAkuBot() {
    if (!IsAimlabInstalled()) {
        MessageBoxW(NULL, L"x001.", L"Hata", MB_OK | MB_ICONERROR);
        return;
    }

    PostMessage(g_hwnd, WM_CLOSE, 0, 0);

    SHELLEXECUTEINFO seiAimlab = { sizeof(seiAimlab) };
    seiAimlab.lpFile = L"steam://rungameid/714010";
    seiAimlab.nShow = SW_SHOWNORMAL;
    ShellExecuteEx(&seiAimlab);

    Sleep(3000);

    SHELLEXECUTEINFO sei = { sizeof(sei) };
    sei.lpFile = L"AkuBot.exe";
    sei.nShow = SW_SHOWNORMAL;

    if (!ShellExecuteEx(&sei)) {
        std::wstring errorMsg = L" x002. (Hata: " + std::to_wstring(GetLastError()) + L")";
        MessageBoxW(NULL, errorMsg.c_str(), L"Hata", MB_OK | MB_ICONERROR);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void OnPaint(HWND hwnd);
void DrawSteamLogo(HDC hdc, int x, int y, int size);
void DrawRoundRect(HDC hdc, int x, int y, int width, int height, int radius, COLORREF color);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    g_hInstance = hInstance;

    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = WINDOW_CLASS_NAME;

    RegisterClassEx(&wc);

    g_hwnd = CreateWindowEx(
        0,
        WINDOW_CLASS_NAME,
        WINDOW_TITLE,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!g_hwnd) return 0;

    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);
    return (int)msg.wParam;
}

void DrawSteamLogo(HDC hdc, int x, int y, int size) {
    Graphics graphics(hdc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    Color bgDark(255, 20, 30, 50);
    Color bgBright(255, 80, 120, 200);
    Color ringColor(255, 100, 180, 255);
    Color logoStart(255, 255, 100, 150);
    Color logoEnd(255, 255, 50, 200);

    LinearGradientBrush bgBrush(Point(x, y), Point(x + size, y + size), bgDark, bgBright);
    graphics.FillEllipse(&bgBrush, x, y, size, size);

    Pen ringPen(ringColor, 1.2f);
    graphics.DrawEllipse(&ringPen, x + 1, y + 1, size - 2, size - 2);

    int centerX = x + size / 2;
    int centerY = y + size / 2;

    GraphicsPath logoPath;
    logoPath.AddEllipse(centerX - size * 0.15f, centerY - size * 0.25f, size * 0.3f, size * 0.2f);
    logoPath.AddEllipse(centerX - size * 0.2f, centerY - size * 0.05f, size * 0.4f, size * 0.25f);
    logoPath.AddEllipse(centerX - size * 0.1f, centerY + size * 0.15f, size * 0.2f, size * 0.15f);

    LinearGradientBrush logoBrush(Point(centerX, centerY - size * 0.3f), Point(centerX, centerY + size * 0.3f), logoStart, logoEnd);
    graphics.FillPath(&logoBrush, &logoPath);

    GraphicsPath glossPath;
    glossPath.AddEllipse(x + size * 0.2f, y + size * 0.15f, size * 0.35f, size * 0.2f);
    PathGradientBrush glossBrush(&glossPath);
    Color surroundColors[] = { Color(0, 255, 255, 255) };
    int colorCount = 1;
    glossBrush.SetSurroundColors(surroundColors, &colorCount);
    glossBrush.SetCenterColor(Color(60, 255, 255, 255));
    graphics.FillPath(&glossBrush, &glossPath);
}

void DrawRoundRect(HDC hdc, int x, int y, int width, int height, int radius, COLORREF color) {
    Graphics graphics(hdc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    GraphicsPath path;
    path.AddLine(x + radius, y, x + width - radius, y);
    path.AddArc(x + width - 2 * radius, y, 2 * radius, 2 * radius, 270, 90);
    path.AddLine(x + width, y + radius, x + width, y + height - radius);
    path.AddArc(x + width - 2 * radius, y + height - 2 * radius, 2 * radius, 2 * radius, 0, 90);
    path.AddLine(x + width - radius, y + height, x + radius, y + height);
    path.AddArc(x, y + height - 2 * radius, 2 * radius, 2 * radius, 90, 90);
    path.AddLine(x, y + height - radius, x, y + radius);
    path.AddArc(x, y, 2 * radius, 2 * radius, 180, 90);
    path.CloseFigure();

    SolidBrush brush(Color(255, GetRValue(color), GetGValue(color), GetBValue(color)));
    graphics.FillPath(&brush, &path);
}

void OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hbmMem = CreateCompatibleBitmap(hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
    HANDLE hOld = SelectObject(hdcMem, hbmMem);

    RECT rc;
    GetClientRect(hwnd, &rc);
    HBRUSH hBrush = CreateSolidBrush(BACKGROUND_COLOR);
    FillRect(hdcMem, &rc, hBrush);
    DeleteObject(hBrush);

    HFONT hFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Arial");
    SelectObject(hdcMem, hFont);
    SetTextColor(hdcMem, RGB(255, 255, 255));
    SetBkMode(hdcMem, TRANSPARENT);

    RECT textRect = { 0, 30, WINDOW_WIDTH, 60 };
    DrawText(hdcMem, L"AkuBot Launcher", -1, &textRect, DT_CENTER | DT_SINGLELINE);
    DeleteObject(hFont);

    hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Arial");
    SelectObject(hdcMem, hFont);

    Graphics graphics(hdcMem);
    SolidBrush textBrush(Color(180, 255, 255, 255));

    StringFormat format;
    format.SetAlignment(StringAlignmentCenter);

    Font font(hdcMem, hFont);
    graphics.DrawString(L"Platform", -1, &font, PointF(WINDOW_WIDTH / 2.0f, 120.0f), &format, &textBrush);
    DeleteObject(hFont);

    DrawSteamLogo(hdcMem, WINDOW_WIDTH / 2 - 15, 150, 30);

    hFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
    SelectObject(hdcMem, hFont);
    SetTextColor(hdcMem, RGB(255, 255, 255));

    textRect = { 0, 220, WINDOW_WIDTH, 250 };
    DrawText(hdcMem, L"Aimlab 26.05.25 Last Update", -1, &textRect, DT_CENTER | DT_SINGLELINE);
    DeleteObject(hFont);

    DrawRoundRect(hdcMem, 125, 380, 150, 35, 10, g_buttonHover ? BUTTON_HOVER_COLOR : BUTTON_COLOR);

    hFont = CreateFont(16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Arial");
    SelectObject(hdcMem, hFont);
    SetTextColor(hdcMem, RGB(255, 255, 255));

    textRect = { 125, 380, 275, 415 };
    DrawText(hdcMem, L"Inject", -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DeleteObject(hFont);

    BitBlt(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hdcMem, 0, 0, SRCCOPY);

    SelectObject(hdcMem, hOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);

    EndPaint(hwnd, &ps);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        break;

    case WM_PAINT:
        OnPaint(hwnd);
        break;

    case WM_GETMINMAXINFO: {
        MINMAXINFO* mmi = (MINMAXINFO*)lParam;
        mmi->ptMinTrackSize.x = WINDOW_WIDTH;
        mmi->ptMinTrackSize.y = WINDOW_HEIGHT;
        mmi->ptMaxTrackSize.x = WINDOW_WIDTH;
        mmi->ptMaxTrackSize.y = WINDOW_HEIGHT;
        return 0;
    }

    case WM_MOUSEMOVE: {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        RECT buttonRect = { 125, 380, 275, 415 };

        bool hover = PtInRect(&buttonRect, pt);
        if (hover != g_buttonHover) {
            g_buttonHover = hover;
            InvalidateRect(hwnd, &buttonRect, FALSE);
            SetCursor(LoadCursor(NULL, hover ? IDC_HAND : IDC_ARROW));
        }
        break;
    }

    case WM_LBUTTONDOWN: {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        RECT buttonRect = { 125, 380, 275, 415 };

        if (PtInRect(&buttonRect, pt)) {
            std::thread([]() {
                RunAkuBot();
                }).detach();
        }
        break;
    }

    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}