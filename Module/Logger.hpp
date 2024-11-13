#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include <filesystem>
#include <windows.h>

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

#define LOG_INFO(message) Logger::getInstance()->info(message)
#define LOG_WARNING(message) Logger::getInstance()->warning(message)
#define LOG_EXPECTION(message) Logger::getInstance()->expection(message)
#define LOG_DEBUG(message) Logger::getInstance()->debug(message)
#define LOG_LOAD(message) Logger::getInstance()->load(message)