#pragma once
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Windows Headers
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

// STL Headers
#include <string>
#include <vector>

// Link Libraries
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

class NetworkManager {
public:
    static bool disableNetwork();
    static bool enableNetwork();
    static bool isNetworkEnabled();
    static bool disableNetworkAPI();
private:
    static bool executeCommand(const std::string& command);
};