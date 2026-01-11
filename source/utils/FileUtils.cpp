#include "FileUtils.h"
#include "../Engine.h"
#include "../ModuleEvents.h"
#include "Log.h"

#include <fstream>

std::string GetDirectoryFromPath(const std::string& filePath)
{
    std::filesystem::path path(filePath);

    std::string directory = path.parent_path().generic_string();

    if (!directory.empty() && directory.back() != '/')
    {
        directory += '/';
    }

    return directory;
}

std::string GetFileExtension(const std::string& filePath)
{
    std::filesystem::path path(filePath);
    std::string ext = path.extension().generic_string();

    if (!ext.empty() && ext[0] == '.')
    {
        ext = ext.substr(1);
    }

    std::transform(ext.begin(), ext.end(), ext.begin(),
        [](unsigned char c) { return std::tolower(c); });

    return ext;
}

std::string GetFileName(const std::string& filePath)
{
    std::filesystem::path path(filePath);
    return path.filename().generic_string();
}

std::string FindFileInDirectory(const std::string& directoryPath, const std::string& fileName)
{
    try
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath))
        {
            if (entry.is_regular_file() && entry.path().filename() == fileName)
            {
                return entry.path().generic_string();
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        LOG("Error searching in directory: %s", e.what());
    }

    return "";
}

std::vector<std::string> GetListDirectoryContents(const std::string& directoryPath, bool recursive)
{
    std::vector<std::string> allContent;

    auto options = std::filesystem::directory_options::skip_permission_denied;

    std::error_code ec;

    if (recursive)
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath, options, ec))
        {
            if (ec) {
                LOG("Warning: Error accessing a file during recursive scan inside %s", directoryPath.c_str());
                ec.clear();
                continue;
            }

            try {
                if (entry.exists(ec) && !ec) {
                    allContent.push_back(entry.path().generic_string());
                }
            }
            catch (...) {
                continue;
            }
        }
    }
    else
    {
        for (const auto& entry : std::filesystem::directory_iterator(directoryPath, options, ec))
        {
            if (ec) {
                ec.clear();
                continue;
            }
            allContent.push_back(entry.path().generic_string());
        }
    }

    if (ec) {
        LOG("Error: Could not open directory %s", directoryPath.c_str());
    }

    return allContent;
}

bool IsFileDirectory(const std::string& directoryPath)
{
    return std::filesystem::is_directory(directoryPath);
}

bool DoesFileExist(const std::string& filePath)
{
    return std::filesystem::exists(filePath);
}

std::string GetPreviousPath(const std::string& directoryPath)
{

    std::filesystem::path path(directoryPath);

    if (path.has_parent_path())
    {
        return path.parent_path().generic_string();
    }

    return path.generic_string();
}

bool DoesFileHasMeta(const std::string& directoryPath)
{
    return std::filesystem::exists(directoryPath + ".meta");
}

std::string GetMetaPath(const std::string& directoryPath)
{
    return directoryPath + ".meta";
}

std::string GetLibraryPath(const UID uid)
{
    std::string uidStr = std::to_string(uid);

    std::string folder = (uidStr.length() >= 2) ? uidStr.substr(0, 2) : "00";

    std::string directoryPath = "Library/" + folder;

    if (!std::filesystem::exists(directoryPath)) CreateDirectory(directoryPath);

    return directoryPath + "/" + uidStr + ".bin";
}

bool CreateDirectory(const std::string& directoryPath)
{
    return std::filesystem::create_directory(directoryPath);
}

int64_t GetLastModificationTime(const std::string& path)
{
    if (!std::filesystem::exists(path)) return 0;

    auto fileTime = std::filesystem::last_write_time(path);

    auto duration = fileTime.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

uint32_t GetFileHash(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return 0;

    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    uint32_t crc = 0xFFFFFFFF;
    for (char c : buffer) {
        crc = crc ^ (unsigned char)c;
        for (int i = 0; i < 8; i++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320;
            else         crc = crc >> 1;
        }
    }
    return ~crc;
}

bool MoveAssetToFolder(const std::string& oldPath, const std::string& destinationFolder)
{
    std::error_code ec;

    std::filesystem::path source(oldPath);
    std::filesystem::path destDir(destinationFolder);
    std::filesystem::path finalDestination = destDir / source.filename();

    if (!std::filesystem::exists(source)) {
        LOG("Error: Source file does not exist: %s", oldPath.c_str());
        return false;
    }

    if (std::filesystem::exists(finalDestination)) {
        LOG("Error: Destination file already exists. Move cancelled: %s", finalDestination.string().c_str());
        return false;
    }

    std::filesystem::rename(source, finalDestination, ec);

    if (ec) {
        LOG("Error moving asset: %s", ec.message().c_str());
        return false;
    }

    Engine::GetInstance().moduleEvents->PublishImmediate(Event(Event::Type::AssetMoved, source.generic_string().c_str(), finalDestination.generic_string().c_str()));

    return true;
}

bool DeletePath(const std::string& path)
{
    std::error_code ec;

    if (!std::filesystem::exists(path))
    {
        LOG("Error: File to delete not found: %s", path.c_str());
        return false;
    }

    bool success = std::filesystem::remove_all(path, ec);

    if (ec)
    {
        LOG("Error deleting asset: %s. Message: %s", path.c_str(), ec.message().c_str());
        return false;
    }

    LOG("Deleted asset successfully: %s", path.c_str());
    return true;

    Engine::GetInstance().moduleEvents->PublishImmediate(Event(Event::Type::AssetMoved, path.c_str()));
}

bool DeleteAsset(const std::string& path)
{
    std::error_code ec;

    if (!std::filesystem::exists(path))
    {
        LOG("Error: File to delete not found: %s", path.c_str());
        return false;
    }

    bool assetDeleted = std::filesystem::remove_all(path, ec);

    if (ec)
    {
        LOG("Error deleting asset: %s. Message: %s", path.c_str(), ec.message().c_str());
        return false;
    }

    if (assetDeleted)
    {
        std::string metaPath = GetMetaPath(path);

        if (std::filesystem::exists(metaPath))
        {
            std::filesystem::remove(metaPath, ec);

            if (ec)
            {
                LOG("Warning: Asset deleted, but failed to delete meta file: %s", metaPath.c_str());
            }
        }
    }

    LOG("Deleted asset successfully: %s", path.c_str());
    return true;

    Engine::GetInstance().moduleEvents->PublishImmediate(Event(Event::Type::AssetMoved, path.c_str()));
}

std::string GetFileNameNoExtension(const std::string& filePath)
{
    std::filesystem::path path(filePath);

    return path.stem().string();
}