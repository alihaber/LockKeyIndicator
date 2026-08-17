#include <windows.h>
#include <tchar.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void UpdateLockStatus(HWND hwnd);

TCHAR szWindowClass[] = _T("LockKeyIndicator");
TCHAR szTitle[] = _T("Lock Key Indicator");

bool capsOn = false, numOn = false, scrollOn = false;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = szWindowClass;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_TOPMOST,
        100, 100, 220, 100, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    SetTimer(hwnd, 1, 200, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int) msg.wParam;
}

void DrawStatus(HDC hdc, RECT rc) {
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0,0,0));
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SelectObject(hdc, hFont);

    TCHAR buf[128];
    wsprintf(buf, _T("Caps: %s    Num: %s    Scroll: %s"),
        capsOn ? _T("A") : _T("a"),
        numOn ? _T("1") : _T("NumOff"),
        scrollOn ? _T("S") : _T("ScrollOff"));
    DrawText(hdc, buf, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void UpdateLockStatus(HWND hwnd) {
    bool newCaps = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
    bool newNum = (GetKeyState(VK_NUMLOCK) & 0x0001) != 0;
    bool newScroll = (GetKeyState(VK_SCROLL) & 0x0001) != 0;
    if (newCaps != capsOn || newNum != numOn || newScroll != scrollOn) {
        capsOn = newCaps;
        numOn = newNum;
        scrollOn = newScroll;
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        DrawStatus(hdc, rc);
        EndPaint(hwnd, &ps);
        break;
    }
    case WM_TIMER:
        UpdateLockStatus(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}