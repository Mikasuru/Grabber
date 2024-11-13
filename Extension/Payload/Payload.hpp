#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#include <winhttp.h>
#include <nlohmann/json.hpp>
#pragma comment(lib, "winhttp.lib")

class Payload {
public:
    static bool SendMessage(const std::string& webhook_url, const std::string& message);

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