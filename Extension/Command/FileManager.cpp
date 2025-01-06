#include "FileManager.hpp"
#include <algorithm>
#include "../Payload/Payload.hpp"

bool FileManager::isSystemPath(const fs::path& path) {
    std::string pathStr = path.string();
    std::transform(pathStr.begin(), pathStr.end(), pathStr.begin(), ::tolower);

    const std::vector<std::string> systemPaths = {
        "windows", "program files", "$recycle.bin",
        "system volume information", "appdata"
    };

    return std::any_of(systemPaths.begin(), systemPaths.end(),
        [&](const std::string& sys) { return pathStr.find(sys) != std::string::npos; });
}

std::string FileManager::formatSize(uintmax_t size) {
    const uintmax_t KB = 1024;
    const uintmax_t MB = KB * 1024;
    const uintmax_t GB = MB * 1024;

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);

    if (size >= GB) {
        ss << (double)size / GB << " GB";
    }
    else if (size >= MB) {
        ss << (double)size / MB << " MB";
    }
    else if (size >= KB) {
        ss << (double)size / KB << " KB";
    }
    else {
        ss << size << " B";
    }

    return ss.str();
}

std::string FileManager::resolvePath(const std::string& path) {
    // Handle empty path
    if (path.empty()) {
        return fs::current_path().string();
    }

    // Handle absolute path
    if (path.length() >= 2 && path[1] == ':') {
        return path;
    }

    // Handle special folders
    char folderPath[MAX_PATH];
    int folderType = -1;

    if (path == "Downloads" || path == "downloads") {
        folderType = CSIDL_PERSONAL;  // Default to Documents
    }
    else if (path == "Desktop" || path == "desktop") {
        folderType = CSIDL_DESKTOP;
    }
    else if (path == "Documents" || path == "documents") {
        folderType = CSIDL_PERSONAL;
    }

    if (folderType != -1 && SUCCEEDED(SHGetFolderPathA(NULL, folderType, NULL, 0, folderPath))) {
        if (path == "Downloads" || path == "downloads") {
            return std::string(folderPath) + "\\Downloads";
        }
        return folderPath;
    }

    // Return as relative path
    return path;
}

void FileManager::buildDirectoryTree(const std::string& path, std::stringstream& output) {
    output << "```\n";
    output << "Location: " << path << "\n";
    output << "----------------------------------------\n";
    buildTreeRecursive(output, path);
    output << "```";
}

void FileManager::buildTreeRecursive(std::stringstream& output, const fs::path& path, const std::string& prefix) {
    try {
        std::vector<fs::directory_entry> entries;
        std::error_code ec;

        // Collect accessible entries
        for (const auto& entry : fs::directory_iterator(path, ec)) {
            if (!ec && !isSystemPath(entry.path())) {
                entries.push_back(entry);
            }
        }

        // Sort entries
        std::sort(entries.begin(), entries.end(),
            [](const auto& a, const auto& b) {
                bool aIsDir = fs::is_directory(a);
                bool bIsDir = fs::is_directory(b);
                return aIsDir > bIsDir || (aIsDir == bIsDir &&
                    a.path().filename() < b.path().filename());
            });

        // Build tree
        for (size_t i = 0; i < entries.size(); i++) {
            const auto& entry = entries[i];
            bool isLast = (i == entries.size() - 1);
            std::string marker = isLast ? "└── " : "├── ";
            std::string nextPrefix = prefix + (isLast ? "    " : "│   ");
            std::string name = entry.path().filename().string();

            if (fs::is_directory(entry, ec)) {
                output << prefix << marker << "[DIR] " << name << "\n";
                buildTreeRecursive(output, entry.path(), nextPrefix);
            }
            else {
                output << prefix << marker << "[FILE] " << name;
                if (!ec) {
                    output << " (" << formatSize(fs::file_size(entry, ec)) << ")";
                }
                output << "\n";
            }
        }
    }
    catch (...) {}
}