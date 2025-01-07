/*
Provides comprehensive file system management including upload/download capabilities, file movement, deletion, and program execution.
It implements security measures including path validation, file type verification, and size limitations.
*/
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <vector>
#include <filesystem>
#include <shellapi.h>
#include "../../Module/Logger.hpp"

#pragma comment(lib, "shell32.lib")
namespace fs = std::filesystem;

class FileManager {
public:
    static bool uploadFile(const std::string& localPath, const std::string& remotePath);
    static bool downloadFile(const std::string& remotePath, const std::string& localPath);
    static bool deleteFile(const std::string& path);
    static bool moveFile(const std::string& sourcePath, const std::string& destPath);
    static bool isAllowedFileType(const std::string& filename);
    static bool isFileSizeAllowed(const std::string& filePath);
    static std::vector<std::string> listFiles(const std::string& path);
    static bool runFile(const std::string& path);
    static bool isExecutableFile(const std::string& path);
private:
    static std::string getFileExtension(const std::string& path);
    static bool isSystemPath(const fs::path& path);
    static constexpr uintmax_t MAX_FILE_SIZE = 100 * 1024 * 1024; // 100MB
};