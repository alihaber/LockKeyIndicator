#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <tchar.h>
#include <winreg.h>
#include <stdio.h>
#include <shellapi.h>
#include <commctrl.h>
#include <string.h>

// Link with common controls
#pragma comment(lib, "comctl32.lib")

// Global Variables
int delaySeconds = 3;
// Runtime delay in seconds (can differ from saved delay until user saves)
int runtimeDelaySeconds = 3;
UINT_PTR hideTimerId = 2;
bool capsOn = false, numOn = false, scrollOn = false;
bool autoStartEnabled = true;
// Default to NOT showing a tray icon unless the config says otherwise
bool trayEnabled = false;
// Opacity percentage for the window (0-100). Default 50%.
int opacityPercent = 70;
NOTIFYICONDATA nid;
// Keyboard hook and main window handle for triggering on key press
HHOOK g_hKeyboardHook = NULL;
HWND g_hwndMain = NULL;

TCHAR szWindowClass[] = _T("LockKeyIndicator");
TCHAR szTitle[] = _T("Lock Key Indicator");

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void UpdateLockStatus(HWND hwnd);
void DrawStatus(HDC hdc, RECT rc);

// ========== Config File Functions ==========
void SaveSettings() {
    FILE* f = fopen("configLockKey.cfg", "w");
    if (f) {
        fprintf(f, "Windows Start: %s\n", autoStartEnabled ? "Yes" : "No");
        fprintf(f, "Delay: %d\n", delaySeconds);
        fprintf(f, "Sistem Tepsisi: %s\n", trayEnabled ? "Yes" : "No");
        fprintf(f, "Opacity: %d\n", opacityPercent);
        // Auto-save removed; opacity changes are saved immediately
        fclose(f);
    }
}

void UpdateOpacityMenuChecks(HWND hwnd) {
    HMENU hMenu = GetMenu(hwnd);
    if (!hMenu) return;
    HMENU hSettings = GetSubMenu(hMenu, 0); // Settings
    if (!hSettings) return;
    HMENU hOpacity = GetSubMenu(hSettings, 3); // Opacity is the 4th item in Settings
    HMENU hDelay = GetSubMenu(hSettings, 1); // Delay is the 2nd item in Settings
    if (!hOpacity) return;

    // Clear checks for 10..15
    for (int id = 10; id <= 15; ++id) {
        CheckMenuItem(hOpacity, id, MF_BYCOMMAND | MF_UNCHECKED);
    }
    // Check the matching opacity
    int idToCheck = 10; // default 50
    if (opacityPercent == 50) idToCheck = 10;
    else if (opacityPercent == 60) idToCheck = 11;
    else if (opacityPercent == 70) idToCheck = 12;
    else if (opacityPercent == 80) idToCheck = 13;
    else if (opacityPercent == 90) idToCheck = 14;
    else if (opacityPercent == 100) idToCheck = 15;
    CheckMenuItem(hOpacity, idToCheck, MF_BYCOMMAND | MF_CHECKED);
    // Remove Auto-save menu entry if present (we auto-save by default now)
    // Ensure any existing item ID 20 is removed
    int count = GetMenuItemCount(hOpacity);
    for (int i = count - 1; i >= 0; --i) {
        MENUITEMINFO mii = { 0 };
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_ID | MIIM_STRING;
        TCHAR buf[128];
        mii.dwTypeData = buf;
        mii.cch = (UINT) (sizeof(buf)/sizeof(TCHAR));
        if (GetMenuItemInfo(hOpacity, i, TRUE, &mii)) {
            if (mii.wID == 20) {
                RemoveMenu(hOpacity, i, MF_BYPOSITION);
            }
        }
    }

    // Update Delay menu checks (IDs 30..38 -> 2..10 seconds)
    if (hDelay) {
        for (int id = 30; id <= 38; ++id) {
            CheckMenuItem(hDelay, id, MF_BYCOMMAND | MF_UNCHECKED);
        }
        int did = 30 + (delaySeconds - 2);
        if (did < 30) did = 30;
        if (did > 38) did = 38;
        CheckMenuItem(hDelay, did, MF_BYCOMMAND | MF_CHECKED);
    }

}

void LoadSettings() {
    FILE* f = fopen("configLockKey.cfg", "r");
    if (f) {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), f)) {
            if (strncmp(buffer, "Windows Start:", 14) == 0) {
                autoStartEnabled = strstr(buffer, "Yes") != NULL;
            } else if (strncmp(buffer, "Delay:", 6) == 0) {
                sscanf(buffer + 6, "%d", &delaySeconds);
                runtimeDelaySeconds = delaySeconds;
            } else if (strncmp(buffer, "Sistem Tepsisi:", 15) == 0) {
                trayEnabled = strstr(buffer, "Yes") != NULL;
            } else if (strncmp(buffer, "Opacity:", 8) == 0) {
                int val = 0;
                sscanf(buffer + 8, "%d", &val);
                if (val >= 0 && val <= 100) opacityPercent = val;
            }
        }
        fclose(f);
    } else {
        SaveSettings(); // Dosya yoksa varsayılan ayarlarla oluştur
    }
}

// ========== Auto-Start Functions ==========
void SetAutoStart(bool enable) {
    HKEY hKey;
    const TCHAR* runKey = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
    const TCHAR* valueName = _T("LockKeyIndicator");
    TCHAR exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);
    
    if (RegOpenKeyEx(HKEY_CURRENT_USER, runKey, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        if (enable) {
            RegSetValueEx(hKey, valueName, 0, REG_SZ, (BYTE*)exePath, (DWORD)((_tcslen(exePath) + 1) * sizeof(TCHAR)));
            autoStartEnabled = true;
        } else {
            RegDeleteValue(hKey, valueName);
            autoStartEnabled = false;
        }
        RegCloseKey(hKey);
    }
}

// ========== Tray Icon Functions ==========
void AddTrayIcon(HWND hwnd) {
    memset(&nid, 0, sizeof(nid));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_APP + 1;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    _tcscpy_s(nid.szTip, sizeof(nid.szTip) / sizeof(TCHAR), _T("Lock Key Indicator"));
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void RemoveTrayIcon() {
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

// ========== Drawing Functions ==========
void DrawStatus(HDC hdc, RECT rc) {
    LOGFONT lf = { 0 };
    lf.lfHeight = 36;
    lf.lfWeight = FW_BOLD;
    _tcscpy_s(lf.lfFaceName, sizeof(lf.lfFaceName) / sizeof(TCHAR), _T("Segoe UI"));
    HFONT hFont = CreateFontIndirect(&lf);
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    int lineHeight = 44;
    RECT rcLine = rc;
    rcLine.top = rc.top;
    rcLine.bottom = rcLine.top + lineHeight;
    int leftMargin = 25;

    SetBkMode(hdc, OPAQUE);
    SetTextColor(hdc, RGB(255, 255, 255));

    // CAPS LOCK
    COLORREF capsBg = capsOn ? RGB(0, 32, 96) : RGB(192, 0, 0);
    SetBkColor(hdc, capsBg);
    TCHAR capsBuf[32];
    wsprintf(capsBuf, _T("Caps: %s"), capsOn ? _T("On") : _T("Off"));
    RECT rcCaps = rcLine;
    rcCaps.left += leftMargin;
    DrawText(hdc, capsBuf, -1, &rcCaps, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // NUM LOCK
    rcLine.top += lineHeight;
    rcLine.bottom += lineHeight;
    COLORREF numBg = numOn ? RGB(0, 32, 96) : RGB(192, 0, 0);
    SetBkColor(hdc, numBg);
    TCHAR numBuf[32];
    wsprintf(numBuf, _T("Num: %s"), numOn ? _T("On") : _T("Off"));
    RECT rcNum = rcLine;
    rcNum.left += leftMargin;
    DrawText(hdc, numBuf, -1, &rcNum, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // SCROLL LOCK
    rcLine.top += lineHeight;
    rcLine.bottom += lineHeight;
    COLORREF scrlBg = scrollOn ? RGB(0, 32, 96) : RGB(192, 0, 0);
    SetBkColor(hdc, scrlBg);
    TCHAR scrlBuf[32];
    wsprintf(scrlBuf, _T("Scroll: %s"), scrollOn ? _T("On") : _T("Off"));
    RECT rcScrl = rcLine;
    rcScrl.left += leftMargin;
    DrawText(hdc, scrlBuf, -1, &rcScrl, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
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
        ShowWindow(hwnd, SW_SHOW);
        SetTimer(hwnd, hideTimerId, runtimeDelaySeconds * 1000, NULL);
    }
}

// ========== Window Procedure ==========
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_CREATE) {
        g_hwndMain = hwnd;
    }
    switch (msg) {
    case WM_CREATE: {
        if (trayEnabled) {
            AddTrayIcon(hwnd);
        }
        // Ensure menu checks reflect loaded settings
        UpdateOpacityMenuChecks(hwnd);
        break;
    }
        case WM_USER + 100: {
            // simulate key press trigger: update and show
            UpdateLockStatus(hwnd);
            break;
        }
    case WM_APP + 1: {
        // Tray icon right-click menu
        if (LOWORD(lParam) == WM_RBUTTONUP) {
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, 1001, _T("Exit"));
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hwnd);
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hMenu);
        } else if (LOWORD(lParam) == WM_LBUTTONUP || LOWORD(lParam) == WM_LBUTTONDBLCLK) {
            // Single (or double) left-click on tray icon -> show main window
            ShowWindow(hwnd, SW_SHOW);
            UpdateWindow(hwnd);
            SetForegroundWindow(hwnd);
            // also refresh menu checks if needed
            UpdateOpacityMenuChecks(hwnd);
        }
        break;
    }
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case 1001: {
            // Tray icon Exit
            RemoveTrayIcon();
            PostQuitMessage(0);
            break;
        }
        case 3: {
            // Settings > Windows Start
            wchar_t configPath[MAX_PATH] = {0};
            GetModuleFileNameW(NULL, configPath, MAX_PATH);
            wchar_t* lastSlash = wcsrchr(configPath, L'\\');
            if (lastSlash) *(lastSlash + 1) = 0;
            wchar_t msg3[1024];
            swprintf_s(msg3, sizeof(msg3)/sizeof(wchar_t),
                L"Windows Start (Başlangıçta Çalıştırma)\n\n"
                L"Bu ayar, bilgisayar başladığında programın otomatik olarak açılıp açılmayacağını denetler.\n\n"
                L"Klasör: %s\n\n"
                L"Evet: Başlangıçta çalıştır\n"
                L"Hayır: Başlangıçta çalıştırma\n\n"
                L"Dilerseniz configLockKey.cfg dosyasından da ayarlayabilirsiniz.",
                configPath);
            int res = MessageBoxW(hwnd, msg3, L"Windows Start", MB_YESNO | MB_ICONQUESTION);
            if (res == IDYES) {
                SetAutoStart(true);
            } else if (res == IDNO) {
                SetAutoStart(false);
            }
            SaveSettings();
            break;
        }
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15: {
            int newOpacity = 50;
            if (LOWORD(wParam) == 10) newOpacity = 50;
            else if (LOWORD(wParam) == 11) newOpacity = 60;
            else if (LOWORD(wParam) == 12) newOpacity = 70;
            else if (LOWORD(wParam) == 13) newOpacity = 80;
            else if (LOWORD(wParam) == 14) newOpacity = 90;
            else if (LOWORD(wParam) == 15) newOpacity = 100;

            // Apply immediately
            BYTE a = (BYTE)((newOpacity * 255) / 100);
            SetLayeredWindowAttributes(hwnd, 0, a, LWA_ALPHA);

            // Save immediately on selection
            opacityPercent = newOpacity;
            SaveSettings();
            UpdateOpacityMenuChecks(hwnd);
            break;
        }

        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
        case 38: {
            int newDelay = 2 + (LOWORD(wParam) - 30);
            if (newDelay < 2) newDelay = 2;
            if (newDelay > 10) newDelay = 10;
            // Apply immediately for runtime; do not overwrite saved until user confirms
            runtimeDelaySeconds = newDelay;
            // If hide timer is active, update it to new runtime value
            KillTimer(hwnd, hideTimerId);
            // If currently showing, restart timer with new delay
            SetTimer(hwnd, hideTimerId, runtimeDelaySeconds * 1000, NULL);

            // Ask whether to persist this change to config
            wchar_t askMsg[256];
            swprintf_s(askMsg, sizeof(askMsg)/sizeof(wchar_t), L"Gecikmeyi %d saniye olarak uygulayıp ayarları kaydetmek ister misiniz?\n\nEvet = Ayarlara kaydet\nHayır = Yalnızca geçici olarak uygula", newDelay);
            int res = MessageBoxW(hwnd, askMsg, L"Gecikme Süresini Ayarla", MB_YESNO | MB_ICONQUESTION);
            if (res == IDYES) {
                delaySeconds = newDelay;
                SaveSettings();
            }
            UpdateOpacityMenuChecks(hwnd);
            break;
        }
        case 5: {
            // Settings > Sistem Tepsisi
            wchar_t configPath[MAX_PATH] = {0};
            GetModuleFileNameW(NULL, configPath, MAX_PATH);
            wchar_t* lastSlash = wcsrchr(configPath, L'\\');
            if (lastSlash) *(lastSlash + 1) = 0;
            wchar_t msg5[1024];
            swprintf_s(msg5, sizeof(msg5)/sizeof(wchar_t),
                L"System Tray (Sistem Tepsisi)\n\n"
                L"Bu ayar, programın Windows sağ alt köşesindeki sistem tepsisinde bir simge göstermesini denetler.\n\n"
                L"Klasör: %s\n\n"
                L"Evet: Sistem tepsisinde simge göster\n"
                L"Hayır: Sistem tepsisinde simge gösterme\n\n"
                L"Dilerseniz configLockKey.cfg dosyasından da ayarlayabilirsiniz.",
                configPath);
            int res = MessageBoxW(hwnd, msg5, L"System Tray", MB_YESNO | MB_ICONQUESTION);
            if (res == IDYES) {
                trayEnabled = true;
                AddTrayIcon(hwnd);
            } else if (res == IDNO) {
                trayEnabled = false;
                RemoveTrayIcon();
            }
            SaveSettings();
            break;
        }
        case 2: {
            // Help > About (Unicode so Turkish characters display correctly)
            const wchar_t* aboutText = L"LockKeyIndicator v1.0\r\n\r\nYazar: Ali HABER\r\nogcizimci@gmail.com\r\n\r\nİnsanların en hayırlısı insanlara faydalı olandır.\r\n\r\n\"Buhari, Megazi, 35\"";
            MessageBoxW(hwnd, aboutText, L"About", MB_OK | MB_ICONINFORMATION);
            break;
        }
        }
        break;
    }
    case WM_TIMER: {
        if (wParam == hideTimerId) {
            ShowWindow(hwnd, SW_HIDE);
            KillTimer(hwnd, hideTimerId);
        } else if (wParam == 1) {
            UpdateLockStatus(hwnd);
        }
        break;
    }
    case WM_CLOSE: {
        // Ask user whether to exit or run in background
        int res = MessageBoxW(hwnd,
            L"Program kapansın mı?\n\nEvet = Uygulamadan çık\nHayır = Arka planda çalışmaya devam et (Sistem Tepsisi simgesinde çalışacak)",
            L"Kapat",
            MB_YESNO | MB_ICONQUESTION);
        if (res == IDYES) {
            // Remove tray icon (if present) and close
            RemoveTrayIcon();
            DestroyWindow(hwnd);
        } else {
            // Hide window and ensure a tray icon exists so the user can reopen the app
            ShowWindow(hwnd, SW_HIDE);
            if (!trayEnabled) {
                trayEnabled = true;
                AddTrayIcon(hwnd);
            }
        }
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        DrawStatus(hdc, rc);
        EndPaint(hwnd, &ps);
        break;
    }
    case WM_DESTROY: {
        if (trayEnabled) {
            RemoveTrayIcon();
        }
        // Unhook keyboard hook
        if (g_hKeyboardHook) {
            UnhookWindowsHookEx(g_hKeyboardHook);
            g_hKeyboardHook = NULL;
        }
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// ========== Main Function ==========
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Load settings and ensure config file exists
    LoadSettings();
    // Single instance: create a named mutex and exit if another instance exists
    HANDLE hMutex = CreateMutex(NULL, FALSE, _T("Local\\LockKeyIndicator_Mutex"));
    if (hMutex && GetLastError() == ERROR_ALREADY_EXISTS) {
        HWND hOther = FindWindow(szWindowClass, szTitle);
        if (hOther) {
            ShowWindow(hOther, SW_SHOW);
            SetForegroundWindow(hOther);
            // Also trigger the indicator in the running instance
            PostMessage(hOther, WM_USER + 100, 0, 0);
        }
        if (hMutex) CloseHandle(hMutex);
        return 0;
    }
    
    // Double-check that config file was created
    FILE* cfgCheck = fopen("configLockKey.cfg", "r");
    if (cfgCheck) {
        fclose(cfgCheck);
    } else {
        SaveSettings(); // Create if doesn't exist
    }

    HICON hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(101), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = szWindowClass;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = hIcon;
    RegisterClass(&wc);

    // Create Menu
    HMENU hMenu = CreateMenu();
    HMENU hSettingsMenu = CreateMenu();
    AppendMenu(hSettingsMenu, MF_STRING, 3, _T("Windows Start"));
    // Delay submenu (2..10 seconds)
    HMENU hDelayMenu = CreateMenu();
    AppendMenu(hDelayMenu, MF_STRING, 30, _T("2 sec"));
    AppendMenu(hDelayMenu, MF_STRING, 31, _T("3 sec"));
    AppendMenu(hDelayMenu, MF_STRING, 32, _T("4 sec"));
    AppendMenu(hDelayMenu, MF_STRING, 33, _T("5 sec"));
    AppendMenu(hDelayMenu, MF_STRING, 34, _T("6 sec"));
    AppendMenu(hDelayMenu, MF_STRING, 35, _T("7 sec"));
    AppendMenu(hDelayMenu, MF_STRING, 36, _T("8 sec"));
    AppendMenu(hDelayMenu, MF_STRING, 37, _T("9 sec"));
    AppendMenu(hDelayMenu, MF_STRING, 38, _T("10 sec"));
    AppendMenu(hSettingsMenu, MF_POPUP, (UINT_PTR)hDelayMenu, _T("Delay"));
    AppendMenu(hSettingsMenu, MF_STRING, 5, _T("Sistem Tepsisi"));
    // Opacity submenu
    HMENU hOpacityMenu = CreateMenu();
    AppendMenu(hOpacityMenu, MF_STRING, 10, _T("Opacity 50%"));
    AppendMenu(hOpacityMenu, MF_STRING, 11, _T("Opacity 60%"));
    AppendMenu(hOpacityMenu, MF_STRING, 12, _T("Opacity 70%"));
    AppendMenu(hOpacityMenu, MF_STRING, 13, _T("Opacity 80%"));
    AppendMenu(hOpacityMenu, MF_STRING, 14, _T("Opacity 90%"));
    AppendMenu(hOpacityMenu, MF_STRING, 15, _T("Opacity 100%"));
    AppendMenu(hOpacityMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hSettingsMenu, MF_POPUP, (UINT_PTR)hOpacityMenu, _T("Opacity"));
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSettingsMenu, _T("Settings"));
    HMENU hHelpMenu = CreateMenu();
    AppendMenu(hHelpMenu, MF_STRING, 2, _T("About"));
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, _T("Help"));

	// Get screen size (ekran genisliği ve yüksekliği) to position the window at bottom center
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int winW = 190;
    int winH = 190;
    int winX = (screenW - winW) / 2;
    int winY = screenH - winH - 40;

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED,
        szWindowClass,
        szTitle,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        winX, winY, winW, winH,
        NULL, NULL, hInstance, NULL);

    SetMenu(hwnd, hMenu);
    // Update menu check marks to reflect loaded settings
    UpdateOpacityMenuChecks(hwnd);
    // Install low-level keyboard hook to trigger display on key events
    g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT {
        if (nCode == HC_ACTION) {
            KBDLLHOOKSTRUCT* k = (KBDLLHOOKSTRUCT*)lParam;
            // trigger on key up/down of lock keys
            if (k->vkCode == VK_CAPITAL || k->vkCode == VK_NUMLOCK || k->vkCode == VK_SCROLL) {
                if (g_hwndMain) PostMessage(g_hwndMain, WM_USER + 100, 0, 0);
            }
        }
        return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
    }, hInstance, 0);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    SetTimer(hwnd, 1, 200, NULL);

    // Apply initial opacity
    // Convert percentage (0-100) to alpha (0-255)
    BYTE alpha = (BYTE)((opacityPercent * 255) / 100);
    SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    if (hMutex) CloseHandle(hMutex);
    return (int)msg.wParam;
}