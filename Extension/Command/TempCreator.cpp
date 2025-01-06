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

        std::string tempPath = getTempPath();
        if (tempPath.empty()) {
            Logger::getInstance()->warning("Failed to get temp path");
            return false;
        }

        const uint64_t chunkSize = 1024 * 1024; // 1MB
        uint64_t remainingSize = totalSize;
        int filesCreated = 0;

        while (remainingSize > 0 && filesCreated < 1000) {
            std::string filePath = tempPath + "\\" + generateRandomFileName();
            uint64_t currentSize = (remainingSize > chunkSize) ? chunkSize : remainingSize;

            createFile(filePath, currentSize);

            remainingSize -= currentSize;
            filesCreated++;

            Logger::getInstance()->info("Created temp file: " + filePath +
                " (" + std::to_string(filesCreated) + " files created)");
        }

        Logger::getInstance()->info("Created " + std::to_string(filesCreated) +
            " temporary files, total size: " + sizeStr);
        return true;
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection("Error creating temp files: " + std::string(e.what()));
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