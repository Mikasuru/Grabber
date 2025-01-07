#include "Logger.hpp"

bool SavingLogs = true;

Logger* Logger::instance = nullptr;

Logger::Logger() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    initializeLogFile();
}

Logger* Logger::getInstance() {
    if (instance == nullptr) {
        instance = new Logger();
    }
    return instance;
}

void Logger::initializeLogFile() {
    if (SavingLogs == false) {
        return;
    }
    std::filesystem::create_directory("Logs");

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_s(&tm, &time);

    const char* months[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    std::string filename = "Logs/Log_" +
        std::to_string(tm.tm_mday) + "-" +
        std::string(months[tm.tm_mon]) + "-" +
        std::to_string(tm.tm_hour) +
        std::to_string(tm.tm_min) +
        std::to_string(tm.tm_sec) +
        ".kukuri";

    logFile.open(filename, std::ios::app);
}

std::string Logger::getLevelString(Level level) {
    switch (level) {
    case Level::INFO: return "INFO";
    case Level::WARNING: return "WARNING";
    case Level::EXPECTION: return "EXPECTION";
    case Level::DEBUG: return "DEBUG";
    case Level::LOAD: return "LOAD";
    default: return "UNKNOWN";
    }
}

void Logger::setConsoleColor(Level level) {
    switch (level) {
    case Level::INFO:
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Cyan
        break;
    case Level::WARNING:
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Yellow
        break;
    case Level::EXPECTION:
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY); // Red
        break;
    case Level::DEBUG:
        SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY); // Green
        break;
    case Level::LOAD:
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY); // Magenta
        break;
    }
}

void Logger::resetConsoleColor() {
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // White
}

std::string Logger::getCurrentDateTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_s(&tm, &time);

    std::string timeStr =
        std::to_string(tm.tm_hour) + ":" +
        std::to_string(tm.tm_min) + ":" +
        std::to_string(tm.tm_sec);

    return timeStr;
}

void Logger::log(Level level, const std::string& message) {
    std::string levelStr = getLevelString(level);
    std::string timeStr = getCurrentDateTime();
    std::string logMessage = "[" + levelStr + "] (" + timeStr + "): " + message + "\n";

    // Console output
    setConsoleColor(level);
    std::cout << "[" << levelStr << "]";
    resetConsoleColor();
    std::cout << " (" << timeStr << "): " << message << "\n";

    // Save to file
    if (SavingLogs == true) {
        if (logFile.is_open()) {
            logFile << logMessage;
            logFile.flush();
        }
    }
}

void Logger::info(const std::string& message) {
    log(Level::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(Level::WARNING, message);
}

void Logger::expection(const std::string& message) {
    log(Level::EXPECTION, message);
}

void Logger::debug(const std::string& message) {
    log(Level::DEBUG, message);
}

void Logger::load(const std::string& message) {
    log(Level::LOAD, message);
}