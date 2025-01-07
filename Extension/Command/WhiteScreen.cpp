/*
Freeze a program
*/
#include "WhiteScreen.hpp"
#include <algorithm>

bool WhiteScreen::freezeProcess(const std::string& processName) {
    try {
        Logger::getInstance()->info("Attempting to freeze process: " + processName);

        DWORD processId = findProcessId(processName);
        if (processId == 0) {
            Logger::getInstance()->warning("Process not found: " + processName);
            return false;
        }

        return suspendProcess(processId);
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection("Error freezing process: " + std::string(e.what()));
        return false;
    }
}

bool WhiteScreen::unfreezeProcess(const std::string& processName) {
    try {
        Logger::getInstance()->info("Attempting to unfreeze process: " + processName);

        DWORD processId = findProcessId(processName);
        if (processId == 0) {
            Logger::getInstance()->warning("Process not found: " + processName);
            return false;
        }

        return resumeProcess(processId);
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection("Error unfreezing process: " + std::string(e.what()));
        return false;
    }
}

DWORD WhiteScreen::findProcessId(const std::string& processName) {
    DWORD processId = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    std::string targetLower = toLower(processName);
    PROCESSENTRY32W processEntry;
    processEntry.dwSize = sizeof(processEntry);

    if (Process32FirstW(snapshot, &processEntry)) {
        do {
            std::wstring wProcessName = processEntry.szExeFile;
            std::string currentProcess(wProcessName.begin(), wProcessName.end());

            if (toLower(currentProcess).find(targetLower) != std::string::npos) {
                processId = processEntry.th32ProcessID;
                Logger::getInstance()->info("Found process: " + currentProcess + " (PID: " + std::to_string(processId) + ")");
                break;
            }
        } while (Process32NextW(snapshot, &processEntry));
    }

    CloseHandle(snapshot);
    return processId;
}

bool WhiteScreen::suspendProcess(DWORD processId) {
    bool success = false;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    THREADENTRY32 threadEntry;
    threadEntry.dwSize = sizeof(THREADENTRY32);

    if (Thread32First(snapshot, &threadEntry)) {
        do {
            if (threadEntry.th32OwnerProcessID == processId) {
                HANDLE threadHandle = OpenThread(THREAD_SUSPEND_RESUME,
                    FALSE,
                    threadEntry.th32ThreadID);

                if (threadHandle != NULL) {
                    if (SuspendThread(threadHandle) != (DWORD)-1) {
                        success = true;
                        Logger::getInstance()->info("Suspended thread: " + std::to_string(threadEntry.th32ThreadID));
                    }
                    CloseHandle(threadHandle);
                }
            }
        } while (Thread32Next(snapshot, &threadEntry));
    }

    CloseHandle(snapshot);
    return success;
}

bool WhiteScreen::resumeProcess(DWORD processId) {
    bool success = false;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    THREADENTRY32 threadEntry;
    threadEntry.dwSize = sizeof(THREADENTRY32);

    if (Thread32First(snapshot, &threadEntry)) {
        do {
            if (threadEntry.th32OwnerProcessID == processId) {
                HANDLE threadHandle = OpenThread(THREAD_SUSPEND_RESUME,
                    FALSE,
                    threadEntry.th32ThreadID);

                if (threadHandle != NULL) {
                    while (ResumeThread(threadHandle) > 0) {
                        success = true;
                        Logger::getInstance()->info("Resumed thread: " + std::to_string(threadEntry.th32ThreadID));
                    }
                    CloseHandle(threadHandle);
                }
            }
        } while (Thread32Next(snapshot, &threadEntry));
    }

    CloseHandle(snapshot);
    return success;
}

std::string WhiteScreen::toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}