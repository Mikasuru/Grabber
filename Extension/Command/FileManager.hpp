#pragma once
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <shlobj.h>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include "../../Module/Logger.hpp"

namespace fs = std::filesystem;

class FileManager {
public:
    struct FileInfo {
        std::string name;
        std::string path;
        bool isDirectory;
        uintmax_t size;
    };

    static std::vector<FileInfo> listFiles(const std::string& path);
    static std::string resolvePath(const std::string& path);
    static void buildDirectoryTree(const std::string& path, std::stringstream& output);

private:
    static bool isSystemPath(const fs::path& path);
    static std::string formatSize(uintmax_t size);
    static void buildTreeRecursive(std::stringstream& output, const fs::path& path, const std::string& prefix = "");
};