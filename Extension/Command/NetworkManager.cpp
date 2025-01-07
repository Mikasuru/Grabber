/*
Controls network interfaces, Such as Internet Driver.
*/
#include "NetworkManager.hpp"
#include "../Module/Logger.hpp"
#include <windows.h>
#include <iostream>
#include <netcon.h>
#include <objbase.h>
#include <iphlpapi.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "iphlpapi.lib")

bool NetworkManager::disableNetwork() {
    bool success = false;

    const char* adapters[] = {
        "Ethernet",
        "Wi-Fi",
        "Local Area Connection",
        "Wireless Network Connection"
    };

    for (const char* adapter : adapters) {
        std::string command = "netsh interface set interface \"";
        command += adapter;
        command += "\" disable";

        STARTUPINFOA si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE; 

        CreateProcessA(NULL, (LPSTR)command.c_str(),
            NULL, NULL, FALSE,
            CREATE_NO_WINDOW, 
            NULL, NULL, &si, &pi);

        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        if (exitCode == 0) {
            success = true;
            Logger::getInstance()->info(std::string("Disabled adapter: ") + adapter);
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    return success;
}

bool NetworkManager::enableNetwork() {
    bool success = false;

    const char* adapters[] = {
        "Ethernet",
        "Wi-Fi",
        "Local Area Connection",
        "Wireless Network Connection"
    };

    for (const char* adapter : adapters) {
        std::string command = "netsh interface set interface \"";
        command += adapter;
        command += "\" enable";

        STARTUPINFOA si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        CreateProcessA(NULL, (LPSTR)command.c_str(),
            NULL, NULL, FALSE,
            CREATE_NO_WINDOW,
            NULL, NULL, &si, &pi);

        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        if (exitCode == 0) {
            success = true;
            Logger::getInstance()->info(std::string("Enabled adapter: ") + adapter);
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    return success;
}