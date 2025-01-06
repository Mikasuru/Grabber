#pragma once
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include <filesystem>

class Logger {
private:
    static Logger* instance;
    std::ofstream logFile;
    HANDLE hConsole;

    Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

public:
    enum class Level {
        INFO,
        WARNING,
        EXPECTION,
        DEBUG,
        LOAD
    };

    static Logger* getInstance();
    void log(Level level, const std::string& message);

    void info(const std::string& message);
    void warning(const std::string& message);
    void expection(const std::string& message);
    void debug(const std::string& message);
    void load(const std::string& message);

private:
    std::string getLevelString(Level level);
    void setConsoleColor(Level level);
    void resetConsoleColor();
    std::string getCurrentDateTime();
    void initializeLogFile();
};
