/*
Handles system crashes, Trigger Blue Screen of Death
*/
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winternl.h>
#include <string>
#include "../../Module/Logger.hpp"

#pragma comment(lib, "ntdll.lib")

extern "C" NTSTATUS NTAPI RtlAdjustPrivilege(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN Enabled);
extern "C" NTSTATUS NTAPI NtRaiseHardError(NTSTATUS ErrorStatus, ULONG NumberOfParameters, ULONG UnicodeStringParameterMask OPTIONAL, PULONG_PTR Parameters, ULONG ResponseOption, PULONG Response);

class BSoDClient {
public:
    static bool triggerBSoDProcess();    // Kill Process
    static bool triggerBSoDDriver();     // Driver Crash

private:
    static bool executeCommand(const std::string& command);
    static bool enablePrivilege(LPCSTR privilege);
};