#pragma once
#include <string>

class SendHardware {
public:
    static bool SendSystemInfo(const std::string& webhook_url);
private:
    static int GetRandomColor();
};