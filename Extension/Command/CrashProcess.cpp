#include "CrashProcess.hpp"
#include <algorithm>

bool CrashProcess::crashProcessByName(const std::string& processName) {
    if (processName.empty()) {
        Logger::getInstance()->warning("Empty process name provided");
        return false;
    }

    Logger::getInstance()->info("Attempting to crash process: " + processName);
    return findAndTerminateProcess(processName);
}

bool CrashProcess::findAndTerminateProcess(const std::string& targetProcess) {
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
                HANDLE processHandle = OpenProcess(PROCESS_TERMINATE,
                    FALSE,
                    processEntry.th32ProcessID);

                if (processHandle != NULL) {
                    if (TerminateProcess(processHandle, 1)) {
                        success = true;
                        Logger::getInstance()->info("Successfully terminated process: " + processNameStr);
                    }
                    else {
                        Logger::getInstance()->warning("Failed to terminate process: " + processNameStr);
                    }
                    CloseHandle(processHandle);
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

std::string CrashProcess::toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}