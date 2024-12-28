#pragma once
#include <windows.h>
#include <vector>

class AudioController {
public:
    static bool ToggleMuteForProcess(DWORD processId);

private:
    static std::vector<DWORD> GetChildProcessIds(DWORD parentProcessId);
    static bool SetMuteStateForProcess(DWORD processId);
    static bool ToggleAudioSessions(struct IAudioSessionEnumerator* sessionEnumerator,
        int sessionCount, DWORD targetProcessId);
};
