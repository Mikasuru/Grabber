/*
Provides comprehensive file system management including upload/download capabilities, file movement, deletion, and program execution.
It implements security measures including path validation, file type verification, and size limitations.
*/
#include "FileManager.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>

bool FileManager::runFile(const std::string& path) {
    try {
        if (!fs::exists(path)) {
            Logger::getInstance()->warning("File does not exist: " + path);
            return false;
        }

        std::string absolutePath = fs::absolute(path).string();
        Logger::getInstance()->info("Attempting to run file: " + absolutePath);

        // Get file extension
        std::string ext = fs::path(path).extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        Logger::getInstance()->info("File extension: " + ext);

        HINSTANCE result = ShellExecuteA(
            NULL,
            "open",
            absolutePath.c_str(),
            NULL,
            NULL,
            SW_SHOWNORMAL
        );

        DWORD error = GetLastError();
        INT_PTR resultValue = (INT_PTR)result;

        if (resultValue > 32) {
            Logger::getInstance()->info("Successfully launched file: " + absolutePath);
            return true;
        }
        else {
            std::string errorMsg;
            switch (resultValue) {
            case 0:
                errorMsg = "Out of memory or resources";
                break;
            case 2:
                errorMsg = "File not found";
                break;
            case 3:
                errorMsg = "Path not found";
                break;
            case 5:
                errorMsg = "Access denied";
                break;
            case 26:
                errorMsg = "Share violation";
                break;
            case 27:
                errorMsg = "Incomplete association";
                break;
            case 28:
                errorMsg = "DDE timeout";
                break;
            case 29:
                errorMsg = "DDE fail";
                break;
            case 30:
                errorMsg = "DDE busy";
                break;
            case 31:
                errorMsg = "No association";
                break;
            default:
                errorMsg = "Unknown error";
            }

            Logger::getInstance()->warning("Failed to launch file. Error: " + errorMsg +
                " (Code: " + std::to_string(resultValue) + ", WinError: " + std::to_string(error) + ")");
            return false;
        }
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection("Error in runFile: " + std::string(e.what()));
        return false;
    }
}

std::string FileManager::getFileExtension(const std::string& path) {
    size_t pos = path.find_last_of(".");
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
    return "";
}

bool FileManager::isExecutableFile(const std::string& path) {
    std::string ext = getFileExtension(path);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == "exe" || ext == "bat" || ext == "cmd";
}

bool FileManager::isSystemPath(const fs::path& path) {
    std::string pathStr = path.string();
    std::transform(pathStr.begin(), pathStr.end(), pathStr.begin(), ::tolower);

    const std::vector<std::string> systemPaths = {
        "\\windows",
        "\\program files",
        "\\$recycle.bin",
        "\\system volume information",
        "\\appdata"
    };

    for (const auto& sysPath : systemPaths) {
        if (pathStr.find(sysPath) != std::string::npos) {
            return true;
        }
    }

    return false;
}

bool FileManager::uploadFile(const std::string& localPath, const std::string& remotePath) {
    try {
        if (isSystemPath(remotePath)) {
            Logger::getInstance()->warning("Cannot upload to system path: " + remotePath);
            return false;
        }

        fs::path remoteDir = fs::path(remotePath).parent_path();
        if (!fs::exists(remoteDir)) {
            fs::create_directories(remoteDir);
        }

        fs::copy_file(localPath, remotePath, fs::copy_options::overwrite_existing);

        Logger::getInstance()->info("Successfully uploaded file from " + localPath + " to " + remotePath);
        return true;
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection("Error uploading file: " + std::string(e.what()));
        return false;
    }
}

bool FileManager::downloadFile(const std::string& remotePath, const std::string& localPath) {
    try {
        if (!fs::exists(remotePath)) {
            Logger::getInstance()->warning("Source file does not exist: " + remotePath);
            return false;
        }

        if (isSystemPath(localPath)) {
            Logger::getInstance()->warning("Cannot download to system path: " + localPath);
            return false;
        }

        fs::path localDir = fs::path(localPath).parent_path();
        if (!fs::exists(localDir)) {
            fs::create_directories(localDir);
        }

        fs::copy_file(remotePath, localPath, fs::copy_options::overwrite_existing);

        Logger::getInstance()->info("Successfully downloaded file from " + remotePath + " to " + localPath);
        return true;
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection("Error downloading file: " + std::string(e.what()));
        return false;
    }
}

bool FileManager::isAllowedFileType(const std::string& filename) {
    std::string ext = fs::path(filename).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    const std::vector<std::string> allowedExtensions = {
        ".txt", ".exe", ".doc", ".docx", ".xls", ".xlsx",
        ".jpg", ".jpeg", ".png", ".gif", ".zip", ".rar"
    };

    return std::find(allowedExtensions.begin(), allowedExtensions.end(), ext)
        != allowedExtensions.end();
}

bool FileManager::isFileSizeAllowed(const std::string& filePath) {
    try {
        const uintmax_t maxSize = 100 * 1024 * 1024; // 100MB
        uintmax_t fileSize = fs::file_size(filePath);
        return fileSize <= maxSize;
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection("Error checking file size: " + std::string(e.what()));
        return false;
    }
}

bool FileManager::deleteFile(const std::string& path) {
    try {
        if (!fs::exists(path)) {
            Logger::getInstance()->warning("File does not exist: " + path);
            return false;
        }

        if (isSystemPath(path)) {
            Logger::getInstance()->warning("Cannot delete system path: " + path);
            return false;
        }

        fs::remove(path);
        Logger::getInstance()->info("Successfully deleted file: " + path);
        return true;
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection("Error deleting file: " + std::string(e.what()));
        return false;
    }
}

std::vector<std::string> FileManager::listFiles(const std::string& path) {
    std::vector<std::string> files;
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (!isSystemPath(entry.path())) {
                files.push_back(entry.path().string());
            }
        }
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection("Error listing files: " + std::string(e.what()));
    }
    return files;
}

bool FileManager::moveFile(const std::string& sourcePath, const std::string& destPath) {
    try {
        if (!fs::exists(sourcePath)) {
            Logger::getInstance()->warning("Source file does not exist: " + sourcePath);
            return false;
        }

        if (isSystemPath(sourcePath) || isSystemPath(destPath)) {
            Logger::getInstance()->warning("Cannot move system files");
            return false;
        }

        fs::rename(sourcePath, destPath);
        Logger::getInstance()->info("Successfully moved file: " + sourcePath + " to " + destPath);
        return true;
    }
    catch (const std::exception& e) {
        Logger::getInstance()->expection("Error moving file: " + std::string(e.what()));
        return false;
    }
}