/*
Manages Discord webhook communications.
These files handle command processing, execution logging, and message delivery.
The system maintains real-time communication between the client and server through structured HTTP requests.
*/
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <winhttp.h>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winhttp.lib")

class Payload {
public:
    static bool SendMessage(const std::string& webhook_url, const std::string& message);
    static bool SendFile(const std::string& webhook_url, const std::string& message, const std::string& file_path);

    struct EmbedField {
        std::string name;
        std::string value;
        bool inline_field = false;
    };

    struct EmbedFooter {
        std::string text;
        std::string icon_url;
    };

    struct Embed {
        std::string title;
        std::string description;
        int color = 0;
        std::vector<EmbedField> fields;
        EmbedFooter footer;
        std::string timestamp;
        std::string thumbnail_url;
    };

    static bool SendEmbed(const std::string& webhook_url, const Embed& embed);

private:
    static bool SendWebhookRequest(const std::string& webhook_url, const std::string& json_payload);
    static std::wstring StringToWString(const std::string& str);
};