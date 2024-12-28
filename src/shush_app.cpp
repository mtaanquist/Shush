#include "../include/shush_app.h"
#include "../include/audio_controller.h"
#include <stdexcept>

ShushApp* ShushApp::instance_ = nullptr;

ShushApp::ShushApp() : hwnd_(nullptr), isHotkeyRegistered_(false), nid_{} {
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
        throw std::runtime_error("Failed to initialize COM");
    }
}

ShushApp::~ShushApp() {
    if (isHotkeyRegistered_) {
        UnregisterHotKey(hwnd_, ID_TOGGLE_MUTE);
    }
    if (nid_.hWnd) {
        Shell_NotifyIcon(NIM_DELETE, &nid_);
    }
    CoUninitialize();
}

void ShushApp::Initialize(HINSTANCE hInstance) {
    RegisterWindowClass(hInstance);
    CreateAppWindow(hInstance);
    RegisterHotkey();
    CreateTrayIcon();
}

void ShushApp::Run() {
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void ShushApp::RegisterWindowClass(HINSTANCE hInstance) {
    const wchar_t CLASS_NAME[] = L"ShushWindowClass";
    WNDCLASS wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc)) {
        throw std::runtime_error("Failed to register window class");
    }
}

void ShushApp::CreateAppWindow(HINSTANCE hInstance) {
    instance_ = this;
    hwnd_ = CreateWindowEx(
        0, L"ShushWindowClass", L"Shush",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hwnd_) {
        throw std::runtime_error("Failed to create window");
    }
}

void ShushApp::RegisterHotkey() {
    isHotkeyRegistered_ = RegisterHotKey(hwnd_, ID_TOGGLE_MUTE,
        MOD_CONTROL | MOD_SHIFT | MOD_WIN, 'M');

    if (!isHotkeyRegistered_) {
        // Just log error, don't throw - app can still work without hotkey
        OutputDebugString(L"Failed to register hotkey Ctrl+Shift+Win+M\n");
    }
}

void ShushApp::CreateTrayIcon() {
    nid_.cbSize = sizeof(NOTIFYICONDATA);
    nid_.hWnd = hwnd_;
    nid_.uID = IDI_TRAY;
    nid_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid_.uCallbackMessage = WM_TRAYICON;
    nid_.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcscpy_s(nid_.szTip, L"Shush - Process Muter");

    if (!Shell_NotifyIcon(NIM_ADD, &nid_)) {
        throw std::runtime_error("Failed to create tray icon");
    }
}

void ShushApp::ShowContextMenu() const {
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    if (hMenu) {
        InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, ID_TRAY_EXIT, L"Exit");
        SetForegroundWindow(hwnd_);
        TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON,
            pt.x, pt.y, 0, hwnd_, nullptr);
        DestroyMenu(hMenu);
    }
}

LRESULT CALLBACK ShushApp::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (instance_) {
        switch (msg) {
        case WM_HOTKEY:
            if (wParam == ID_TOGGLE_MUTE) {
                HWND foregroundWindow = GetForegroundWindow();
                if (foregroundWindow) {
                    DWORD processId;
                    GetWindowThreadProcessId(foregroundWindow, &processId);
                    AudioController::ToggleMuteForProcess(processId);
                }
            }
            return 0;

        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP) {
                instance_->ShowContextMenu();
            }
            return 0;

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_TRAY_EXIT) {
                DestroyWindow(hwnd);
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
