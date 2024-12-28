#include "../include/audio_controller.h"
#include "../include/windows_handle.h"
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <tlhelp32.h>

bool AudioController::ToggleMuteForProcess(DWORD processId) {
    std::vector<DWORD> processIds = GetChildProcessIds(processId);
    processIds.push_back(processId);

    for (DWORD pid : processIds) {
        if (SetMuteStateForProcess(pid)) {
            return true;
        }
    }
    return false;
}

std::vector<DWORD> AudioController::GetChildProcessIds(DWORD parentProcessId) {
    std::vector<DWORD> childProcessIds;
    WindowsHandle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
    if (snapshot == INVALID_HANDLE_VALUE) return childProcessIds;

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(pe32);

    if (Process32FirstW(snapshot, &pe32)) {
        do {
            if (pe32.th32ParentProcessID == parentProcessId) {
                childProcessIds.push_back(pe32.th32ProcessID);
                auto grandChildren = GetChildProcessIds(pe32.th32ProcessID);
                childProcessIds.insert(childProcessIds.end(), grandChildren.begin(), grandChildren.end());
            }
        } while (Process32NextW(snapshot, &pe32));
    }

    return childProcessIds;
}

bool AudioController::SetMuteStateForProcess(DWORD processId) {
    bool found = false;
    IMMDeviceEnumerator* deviceEnumerator = nullptr;
    IMMDevice* defaultDevice = nullptr;
    IAudioSessionManager2* sessionManager = nullptr;
    IAudioSessionEnumerator* sessionEnumerator = nullptr;

    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
    if (FAILED(hr)) return false;

    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &defaultDevice);
    if (SUCCEEDED(hr)) {
        hr = defaultDevice->Activate(__uuidof(IAudioSessionManager2),
            CLSCTX_ALL, nullptr, (void**)&sessionManager);
        if (SUCCEEDED(hr)) {
            hr = sessionManager->GetSessionEnumerator(&sessionEnumerator);
            if (SUCCEEDED(hr)) {
                int sessionCount;
                hr = sessionEnumerator->GetCount(&sessionCount);
                if (SUCCEEDED(hr)) {
                    found = ToggleAudioSessions(sessionEnumerator, sessionCount, processId);
                }
            }
        }
    }

    if (sessionEnumerator) sessionEnumerator->Release();
    if (sessionManager) sessionManager->Release();
    if (defaultDevice) defaultDevice->Release();
    if (deviceEnumerator) deviceEnumerator->Release();

    return found;
}

bool AudioController::ToggleAudioSessions(IAudioSessionEnumerator* sessionEnumerator,
    int sessionCount, DWORD targetProcessId) {
    for (int i = 0; i < sessionCount; i++) {
        IAudioSessionControl* audioSession = nullptr;
        IAudioSessionControl2* sessionControl2 = nullptr;
        ISimpleAudioVolume* volumeControl = nullptr;
        bool found = false;

        HRESULT hr = sessionEnumerator->GetSession(i, &audioSession);
        if (SUCCEEDED(hr)) {
            hr = audioSession->QueryInterface(__uuidof(IAudioSessionControl2),
                (void**)&sessionControl2);
            if (SUCCEEDED(hr)) {
                DWORD sessionProcessId;
                hr = sessionControl2->GetProcessId(&sessionProcessId);
                if (SUCCEEDED(hr) && sessionProcessId == targetProcessId) {
                    hr = audioSession->QueryInterface(__uuidof(ISimpleAudioVolume),
                        (void**)&volumeControl);
                    if (SUCCEEDED(hr)) {
                        BOOL isMuted;
                        hr = volumeControl->GetMute(&isMuted);
                        if (SUCCEEDED(hr)) {
                            volumeControl->SetMute(!isMuted, nullptr);
                            found = true;
                        }
                        volumeControl->Release();
                    }
                }
                sessionControl2->Release();
            }
            audioSession->Release();
        }
        if (found) return true;
    }
    return false;
}
