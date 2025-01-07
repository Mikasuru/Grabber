/*
Manages temporary file operations.
*/
#include "TempCreator.hpp"
#include <random>
#include <sstream>

bool TempCreator::createTempFiles(const std::string& sizeStr) {
    try {
        uint64_t totalSize = parseSize(sizeStr);
        if (totalSize == 0) {
            Logger::getInstance()->warning("Invalid size format: " + sizeStr);
            return false;
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1000, 9999); // random 4 digits
        int randomNum = dis(gen);

        std::string tempPath = getTempPath() + "CGMalware_" + std::to_string(randomNum);
        Logger::getInstance()->info("Creating temp file at: " + tempPath);

        std::string command = "fsutil file createnew \"" + tempPath + "\" " + std::to_string(totalSize);

        STARTUPINFOA si = { sizeof(STARTUPINFOA) };
        PROCESS_INFORMATION pi;

        si.dwFlags |= STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        if (CreateProcessA(NULL, (LPSTR)command.c_str(),
            NULL, NULL, FALSE, CREATE_NO_WINDOW,
            NULL, NULL, &si, &pi)) {

            WaitForSingleObject(pi.hProcess, INFINITE);

            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            if (exitCode == 0) {
                Logger::getInstance()->info("Successfully created temp file: CGMalware_" +
                    std::to_string(randomNum) + " with size: " + sizeStr);
                return true;
            }
        }

        DWORD error = GetLastError();
        Logger::getInstance()->warning("Failed to create temp file. Error code: " + std::to_string(error));
        return false;
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection("Error creating temp file: " + std::string(e.what()));
        return false;
    }
}

std::string TempCreator::getTempPath() {
    char tempPath[MAX_PATH];
    DWORD result = GetTempPathA(MAX_PATH, tempPath);
    if (result == 0) {
        return "";
    }
    return std::string(tempPath);
}

uint64_t TempCreator::parseSize(const std::string& sizeStr) {
    try {
        double size = std::stod(sizeStr.substr(0, sizeStr.length() - 2));
        std::string unit = sizeStr.substr(sizeStr.length() - 2);

        if (unit == "MB") {
            return static_cast<uint64_t>(size * 1024 * 1024);
        }
        else if (unit == "GB") {
            return static_cast<uint64_t>(size * 1024 * 1024 * 1024);
        }
    }
    catch (...) {
        return 0;
    }
    return 0;
}

void TempCreator::createFile(const std::string& path, uint64_t size) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to create file: " + path);
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    const size_t bufferSize = 8192; // 8KB buffer
    std::vector<char> buffer(bufferSize);

    uint64_t remainingSize = size;
    while (remainingSize > 0) {
        size_t currentSize = (remainingSize > bufferSize) ? bufferSize : static_cast<size_t>(remainingSize);

        for (size_t i = 0; i < currentSize; ++i) {
            buffer[i] = static_cast<char>(dis(gen));
        }

        file.write(buffer.data(), currentSize);
        remainingSize -= currentSize;
    }

    file.close();
}

std::string TempCreator::generateRandomFileName() {
    static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);

    std::string filename = "temp_";
    for (int i = 0; i < 10; ++i) {
        filename += charset[dis(gen)];
    }
    filename += ".tmp";
    return filename;
}