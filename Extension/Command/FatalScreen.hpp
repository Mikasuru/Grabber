/*
Born to show a picture, Forced to only restart.
Show a picture and restart system.
While showing a picture, It will disable Superkey
*/
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>
#include <shellapi.h>
#include <string>
#include "../../Module/Logger.hpp"

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "wininet.lib")

class FatalScreen {
public:
    static bool showFatalScreen(bool autoRestart = true, int delaySeconds = 4);
    static void enableTaskManager();

private:
    static void disableTaskManager();
};