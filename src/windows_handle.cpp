#include "../include/windows_handle.h"

WindowsHandle::WindowsHandle(HANDLE handle) : handle_(handle) {}

WindowsHandle::~WindowsHandle() {
    if (handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(handle_);
    }
}

WindowsHandle::operator HANDLE() const {
    return handle_;
}
