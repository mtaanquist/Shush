#pragma once
#include <windows.h>

class WindowsHandle {
public:
    WindowsHandle(HANDLE handle);
    ~WindowsHandle();
    operator HANDLE() const;
private:
    HANDLE handle_;
    WindowsHandle(const WindowsHandle&) = delete;
    WindowsHandle& operator=(const WindowsHandle&) = delete;
};
