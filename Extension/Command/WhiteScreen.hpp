/*
Freeze a program
*/
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <TlHelp32.h>
#include <string>
#include "../../Module/Logger.hpp"

class WhiteScreen {
public:
    static bool freezeProcess(const std::string& processName);
    static bool unfreezeProcess(const std::string& processName);
private:
    static DWORD findProcessId(const std::string& processName);
    static bool suspendProcess(DWORD processId);
    static bool resumeProcess(DWORD processId);
    static std::string toLower(const std::string& str);
};