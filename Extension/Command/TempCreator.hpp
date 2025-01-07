/*
Manages temporary file operations.
*/
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <filesystem>
#include "../../Module/Logger.hpp"

class TempCreator {
public:
    static bool createTempFiles(const std::string& sizeStr);
private:
    static std::string getTempPath();
    static uint64_t parseSize(const std::string& sizeStr);
    static void createFile(const std::string& path, uint64_t size);
    static std::string generateRandomFileName();
};