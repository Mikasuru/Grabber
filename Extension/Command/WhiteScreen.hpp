#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <TlHelp32.h>
#include <string>
#include "../../Module/Logger.hpp"

class WhiteScreen {
public:
    static bool makeProcessUnresponsive(const std::string& processName);

private:
    static bool findAndSuspendProcess(const std::string& processName);
    static std::string toLower(const std::string& str);
};