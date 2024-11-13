#include "Payload.hpp"

bool Payload::SendMessage(const std::string& webhook_url, const std::string& message) {
    nlohmann::json payload = {
        {"content", message}
    };

    return SendWebhookRequest(webhook_url, payload.dump());
}

bool Payload::SendEmbed(const std::string& webhook_url, const Embed& embed) {
    nlohmann::json embedJson;

    // General infomation
    if (!embed.title.empty()) embedJson["title"] = embed.title;
    if (!embed.description.empty()) embedJson["description"] = embed.description;
    if (embed.color != 0) embedJson["color"] = embed.color;
    if (!embed.timestamp.empty()) embedJson["timestamp"] = embed.timestamp;
    if (!embed.thumbnail_url.empty()) embedJson["thumbnail"]["url"] = embed.thumbnail_url;

    // Fields
    if (!embed.fields.empty()) {
        nlohmann::json fieldsJson = nlohmann::json::array();
        for (const auto& field : embed.fields) {
            fieldsJson.push_back({
                {"name", field.name},
                {"value", field.value},
                {"inline", field.inline_field}
                });
        }
        embedJson["fields"] = fieldsJson;
    }

    // Footer
    if (!embed.footer.text.empty()) {
        nlohmann::json footerJson = { {"text", embed.footer.text} };
        if (!embed.footer.icon_url.empty()) {
            footerJson["icon_url"] = embed.footer.icon_url;
        }
        embedJson["footer"] = footerJson;
    }

    // Payload
    nlohmann::json payload = {
        {"embeds", nlohmann::json::array({embedJson})}
    };

    return SendWebhookRequest(webhook_url, payload.dump());
}

bool Payload::SendWebhookRequest(const std::string& webhook_url, const std::string& json_payload) {
    // URL to wstring
    std::wstring wide_url = StringToWString(webhook_url);

    // Starting connection
    HINTERNET hSession = WinHttpOpen(L"Discord Webhook Client",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);

    if (!hSession) return false;

    // URL components
    URL_COMPONENTS urlComp = { sizeof(URL_COMPONENTS) };
    urlComp.dwSchemeLength = -1;
    urlComp.dwHostNameLength = -1;
    urlComp.dwUrlPathLength = -1;

    if (!WinHttpCrackUrl(wide_url.c_str(), 0, 0, &urlComp)) {
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Connection
    HINTERNET hConnect = WinHttpConnect(hSession,
        std::wstring(urlComp.lpszHostName, urlComp.dwHostNameLength).c_str(),
        urlComp.nPort,
        0);

    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Request
    HINTERNET hRequest = WinHttpOpenRequest(hConnect,
        L"POST",
        std::wstring(urlComp.lpszUrlPath, urlComp.dwUrlPathLength).c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);

    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Headers
    if (!WinHttpAddRequestHeaders(hRequest,
        L"Content-Type: application/json",
        -1,
        WINHTTP_ADDREQ_FLAG_ADD)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Send Request
    if (!WinHttpSendRequest(hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        (LPVOID)json_payload.c_str(),
        json_payload.length(),
        json_payload.length(),
        0)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Get Response
    if (!WinHttpReceiveResponse(hRequest, nullptr)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Close all connection
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return true;
}

std::wstring Payload::StringToWString(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size_needed);
    return wstr;
}