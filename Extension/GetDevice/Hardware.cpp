#include "Hardware.hpp"
#include <vector>
#include <sstream>
#include <iomanip>
#include <filesystem>

std::string Hardware::GetUsername() {
    char username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    if (GetUserNameA(username, &username_len)) {
        return std::string(username);
    }
    return "Unknown";
}

std::string Hardware::GetIPAddress() {
    PIP_ADAPTER_INFO adapterInfo = nullptr;
    ULONG size = sizeof(IP_ADAPTER_INFO);

    // First get the size needed
    adapterInfo = (IP_ADAPTER_INFO*)malloc(size);
    if (GetAdaptersInfo(adapterInfo, &size) == ERROR_BUFFER_OVERFLOW) {
        free(adapterInfo);
        adapterInfo = (IP_ADAPTER_INFO*)malloc(size);
    }

    std::string ipAddress = "Unknown";
    if (adapterInfo && GetAdaptersInfo(adapterInfo, &size) == NO_ERROR) {
        PIP_ADAPTER_INFO currentAdapter = adapterInfo;
        while (currentAdapter) {
            // Checking Adapter
            if (currentAdapter->Type != MIB_IF_TYPE_LOOPBACK &&
                strcmp(currentAdapter->IpAddressList.IpAddress.String, "0.0.0.0") != 0) {
                ipAddress = currentAdapter->IpAddressList.IpAddress.String;
                // If found IP (not local and 0.0.0.0)
                if (ipAddress != "127.0.0.1" && ipAddress != "0.0.0.0") {
                    break;
                }
            }
            currentAdapter = currentAdapter->Next;
        }
        free(adapterInfo);
    }

    return ipAddress;
}

std::string Hardware::GetMachineID() {
    PIP_ADAPTER_INFO adapterInfo = nullptr;
    ULONG size = sizeof(IP_ADAPTER_INFO);

    // First get the size needed
    adapterInfo = (IP_ADAPTER_INFO*)malloc(size);
    if (GetAdaptersInfo(adapterInfo, &size) == ERROR_BUFFER_OVERFLOW) {
        free(adapterInfo);
        adapterInfo = (IP_ADAPTER_INFO*)malloc(size);
    }

    std::string macAddress = "Unknown";
    if (adapterInfo && GetAdaptersInfo(adapterInfo, &size) == NO_ERROR) {
        PIP_ADAPTER_INFO currentAdapter = adapterInfo;
        while (currentAdapter) {
            if (currentAdapter->Type != MIB_IF_TYPE_LOOPBACK &&
                strcmp(currentAdapter->IpAddressList.IpAddress.String, "0.0.0.0") != 0) {
                char mac[18];
                sprintf_s(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
                    currentAdapter->Address[0], currentAdapter->Address[1],
                    currentAdapter->Address[2], currentAdapter->Address[3],
                    currentAdapter->Address[4], currentAdapter->Address[5]);
                macAddress = mac;
                break;
            }
            currentAdapter = currentAdapter->Next;
        }
        free(adapterInfo);
    }

    return macAddress;
}

/*

RID-500: Administrator account (default)
RID-501: Guest account
RID-1000+: New user accounts

RID-1001: User Account (Not default admin or guest)

*/
std::string Hardware::GetRelativeIdentifier() {
    HANDLE hToken;
    std::string rid = "Unknown";

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        DWORD tokenInfoLength = 0;
        GetTokenInformation(hToken, TokenUser, nullptr, 0, &tokenInfoLength);

        if (tokenInfoLength > 0) {
            std::vector<BYTE> tokenInfo(tokenInfoLength);
            PTOKEN_USER tokenUser = reinterpret_cast<PTOKEN_USER>(&tokenInfo[0]);

            if (GetTokenInformation(hToken, TokenUser, tokenUser, tokenInfoLength, &tokenInfoLength)) {
                DWORD ridValue = *GetSidSubAuthority(tokenUser->User.Sid,
                    *GetSidSubAuthorityCount(tokenUser->User.Sid) - 1);

                std::stringstream ss;
                ss << "RID-" << ridValue;
                rid = ss.str();
            }
        }
        CloseHandle(hToken);
    }

    return rid;
}

std::string Hardware::GetUserProfilePicture() {
    try {
        wchar_t userProfilePath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, userProfilePath))) {
            std::filesystem::path accountPicPath = std::wstring(userProfilePath) + L"\\AppData\\Roaming\\Microsoft\\Windows\\AccountPictures";

            // Checking possible path
            std::vector<std::filesystem::path> possiblePaths = {
                accountPicPath / "UserImage.jpg",
                accountPicPath / "UserImage.png",
                std::filesystem::path(userProfilePath) / "Pictures" / "ContactPhoto.jpg",
                "C:\\ProgramData\\Microsoft\\User Account Pictures\\user.png",
                "C:\\ProgramData\\Microsoft\\User Account Pictures\\Guest.png"
            };

            for (const auto& path : possiblePaths) {
                if (std::filesystem::exists(path)) {
                    return path.string();
                }
            }
        }
    }
    catch (const std::exception&) {
        // Handle any filesystem errors
    }

    return "";
}

std::string Hardware::GetWindowsVersion() {
    std::string version = "Windows ";
    HKEY hKey;
    const char* subKey = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char buffer[256];
        DWORD bufferSize = sizeof(buffer);

        // Get DisplayVersion (Windows Settings)
        if (RegQueryValueExA(hKey, "DisplayVersion", NULL, NULL,
            reinterpret_cast<LPBYTE>(buffer), &bufferSize) == ERROR_SUCCESS) {
            version += buffer;
        }
        // If DisplayVersion doesnt found then try CurrentBuildNumber
        else {
            bufferSize = sizeof(buffer);
            if (RegQueryValueExA(hKey, "CurrentBuildNumber", NULL, NULL,
                reinterpret_cast<LPBYTE>(buffer), &bufferSize) == ERROR_SUCCESS) {
                int buildNumber = atoi(buffer);
                // Check build number
                if (buildNumber >= 22000) {
                    version += "11";
                }
                else if (buildNumber >= 10240) {
                    version += "10";
                }
                else {
                    version += "Unknown";
                }
            }
        }

        RegCloseKey(hKey);
    }

    // Check is Windows Server
    if (IsWindowsServer()) {
        version += " Server";
    }

    // Get build info
    std::string buildInfo = GetWindowsBuildInfo();
    if (!buildInfo.empty()) {
        version += " " + buildInfo;
    }

    return version;
}

bool Hardware::IsWindowsServer() {
    OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0, 0, VER_NT_WORKSTATION };
    DWORDLONG const dwlConditionMask = VerSetConditionMask(0, VER_PRODUCT_TYPE, VER_EQUAL);
    return !VerifyVersionInfoW(&osvi, VER_PRODUCT_TYPE, dwlConditionMask);
}

std::string Hardware::GetWindowsBuildInfo() {
    std::string edition;
    HKEY hKey;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        char buffer[256];
        DWORD bufferSize = sizeof(buffer);

        // Get Edition
        if (RegQueryValueExA(hKey, "EditionID", NULL, NULL,
            reinterpret_cast<LPBYTE>(buffer), &bufferSize) == ERROR_SUCCESS) {
            edition = buffer;

            // Check for specific editions
            if (edition.find("Enterprise") != std::string::npos) {
                if (edition.find("LTSC") != std::string::npos) {
                    edition = "LTSC";
                }
                else {
                    edition = "Enterprise";
                }
            }
            else if (edition.find("Professional") != std::string::npos) {
                edition = "Pro";
            }
            else if (edition.find("Core") != std::string::npos) {
                edition = "Home";
            }
        }

        RegCloseKey(hKey);
    }

    return edition;
}

std::string Hardware::GetWindowsKey() {
    std::string productKey;
    HKEY hKey;
    const char* keyPath = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SoftwareProtectionPlatform";

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char buffer[256];
        DWORD bufferSize = sizeof(buffer);

        if (RegQueryValueExA(hKey, "BackupProductKeyDefault", NULL, NULL,
            reinterpret_cast<LPBYTE>(buffer), &bufferSize) == ERROR_SUCCESS) {
            productKey = buffer;
        }

        RegCloseKey(hKey);
    }

    // Try another way
    if (productKey.empty()) {
        const char* keyPath2 = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\DefaultProductKey";
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyPath2, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            char buffer[256];
            DWORD bufferSize = sizeof(buffer);

            if (RegQueryValueExA(hKey, "ProductKey", NULL, NULL,
                reinterpret_cast<LPBYTE>(buffer), &bufferSize) == ERROR_SUCCESS) {
                productKey = buffer;
            }

            RegCloseKey(hKey);
        }
    }

    return productKey.empty() ? "Unknown" : productKey;
}