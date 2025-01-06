#include "WhiteScreen.hpp"
#include <algorithm>

bool WhiteScreen::makeProcessUnresponsive(const std::string& processName) {
    try {
        Logger::getInstance()->info("Attempting to make process unresponsive: " + processName);
        return findAndSuspendProcess(processName);
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection(std::string("Error in makeProcessUnresponsive: ") + e.what());
        return false;
    }
}

bool WhiteScreen::findAndSuspendProcess(const std::string& targetProcess) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        Logger::getInstance()->expection("Failed to create process snapshot");
        return false;
    }

    std::string targetLower = toLower(targetProcess);
    bool found = false;
    bool success = false;

    PROCESSENTRY32W processEntry;
    processEntry.dwSize = sizeof(processEntry);

    if (Process32FirstW(snapshot, &processEntry)) {
        do {
            std::wstring processName = processEntry.szExeFile;
            std::string processNameStr(processName.begin(), processName.end());
            std::string processNameLower = toLower(processNameStr);

            if (processNameLower == targetLower + ".exe" ||
                processNameLower.find(targetLower) != std::string::npos) {

                found = true;

                // เปิด process ด้วยสิทธิ์การระงับ thread
                HANDLE processHandle = OpenProcess(PROCESS_SUSPEND_RESUME | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION,
                    FALSE,
                    processEntry.th32ProcessID);

                if (processHandle != NULL) {
                    // หา threads ทั้งหมดของ process
                    HANDLE threadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
                    if (threadSnapshot != INVALID_HANDLE_VALUE) {
                        THREADENTRY32 threadEntry;
                        threadEntry.dwSize = sizeof(THREADENTRY32);

                        if (Thread32First(threadSnapshot, &threadEntry)) {
                            do {
                                if (threadEntry.th32OwnerProcessID == processEntry.th32ProcessID) {
                                    // เปิด thread และระงับการทำงาน
                                    HANDLE threadHandle = OpenThread(THREAD_SUSPEND_RESUME,
                                        FALSE,
                                        threadEntry.th32ThreadID);

                                    if (threadHandle != NULL) {
                                        SuspendThread(threadHandle);
                                        CloseHandle(threadHandle);
                                        success = true;
                                    }
                                }
                            } while (Thread32Next(threadSnapshot, &threadEntry));
                        }
                        CloseHandle(threadSnapshot);
                    }
                    CloseHandle(processHandle);
                }

                if (success) {
                    Logger::getInstance()->info("Successfully suspended threads for: " + processNameStr);
                }
                else {
                    Logger::getInstance()->warning("Failed to suspend threads for: " + processNameStr);
                }
            }
        } while (Process32NextW(snapshot, &processEntry));
    }

    CloseHandle(snapshot);

    if (!found) {
        Logger::getInstance()->warning("Process not found: " + targetProcess);
    }

    return success;
}

std::string WhiteScreen::toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}