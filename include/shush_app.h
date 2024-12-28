#pragma once
#include <windows.h>
#include <shellapi.h>

class ShushApp {
public:
    ShushApp();
    ~ShushApp();
    void Initialize(HINSTANCE hInstance);
    void Run();

private:
    static constexpr UINT WM_TRAYICON = WM_USER + 1;
    static constexpr UINT IDI_TRAY = 1;
    static constexpr UINT ID_TRAY_EXIT = 1000;
    static constexpr UINT ID_TOGGLE_MUTE = 1001;

    HWND hwnd_;
    NOTIFYICONDATA nid_;
    bool isHotkeyRegistered_;
    static ShushApp* instance_;

    void RegisterWindowClass(HINSTANCE hInstance);
    void CreateAppWindow(HINSTANCE hInstance);
    void RegisterHotkey();
    void CreateTrayIcon();
    void ShowContextMenu() const;
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
