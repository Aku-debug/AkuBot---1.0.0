#include <Windows.h>
#include <TlHelp32.h>
#include <psapi.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "psapi.lib")

// Renk algýlama fonksiyonlarý
COLORREF GetPixelColor(int x, int y) {
    HDC hdc = GetDC(NULL);
    COLORREF color = GetPixel(hdc, x, y);
    ReleaseDC(NULL, hdc);
    return color;
}

// Fare týklama fonksiyonu
void ClickMouse() {
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(2, inputs, sizeof(INPUT));
}

// Renk karþýlaþtýrma fonksiyonu
bool IsColorClose(COLORREF a, COLORREF b, int tolerance) {
    return abs(GetRValue(a) - GetRValue(b)) <= tolerance &&
        abs(GetGValue(a) - GetGValue(b)) <= tolerance &&
        abs(GetBValue(a) - GetBValue(b)) <= tolerance;
}

// AimLab kontrol fonksiyonu
bool IsAimLabRunning() {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return false;

    DWORD processID;
    GetWindowThreadProcessId(hwnd, &processID);

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (!hProcess) return false;

    char processName[MAX_PATH] = "<unknown>";
    GetModuleFileNameExA(hProcess, NULL, processName, MAX_PATH);
    CloseHandle(hProcess);

    return _stricmp(PathFindFileNameA(processName), "AimLab_tb.exe") == 0;
}

int main() {
    const COLORREF targetColor = RGB(0, 0, 0); // Siyah renk
    const int tolerance = 30; // Renk toleransý
    const POINT center = { 960, 540 }; // Ekran merkezi

    // Ana döngü
    while (true) {
        if (IsAimLabRunning()) {
            COLORREF pixelColor = GetPixelColor(center.x, center.y);
            if (IsColorClose(pixelColor, targetColor, tolerance)) {
                ClickMouse();
                Sleep(10); // Çift týklamayý önlemek için
            }
        }
        Sleep(1); // CPU kullanýmýný azaltmak için
    }

    return 0;
}