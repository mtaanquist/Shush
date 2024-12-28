#include "../include/shush_app.h"
#include <stdexcept>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int) {
    try {
        ShushApp app;
        app.Initialize(hInstance);
        app.Run();
        return 0;
    }
    catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR);
        return 1;
    }
}
