#pragma once
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Windows Headers 
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <Lmcons.h>
#include <Shlobj.h>
#include <ShlObj_core.h>
#include <VersionHelpers.h>

// STL Headers
#include <string>
#include <vector>

// Link Libraries
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Userenv.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Version.lib")

class Hardware {
public:
    // Existing functions
    static std::string GetUsername();
    static std::string GetIPAddress();
    static std::string GetMachineID();
    static std::string GetRelativeIdentifier();
    static std::string GetUserProfilePicture();

    // New functions
    static std::string GetWindowsVersion();
    static std::string GetWindowsKey();

private:
    static std::string GetWindowsBuildInfo();
    static bool IsWindowsServer();
};