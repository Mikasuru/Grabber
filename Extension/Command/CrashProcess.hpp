#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <TlHelp32.h>
#include <string>
#include "../../Module/Logger.hpp"

class CrashProcess {
public:
    static bool crashProcessByName(const std::string& processName);
private:
    static bool findAndTerminateProcess(const std::string& targetProcess);
    static std::string toLower(const std::string& str);
};