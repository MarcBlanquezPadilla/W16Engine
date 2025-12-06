#include "FileUtils.h"
#include "Log.h"

std::string GetDirectoryFromPath(const std::string& filePath)
{
    std::filesystem::path path(filePath);
    return path.parent_path().generic_string();
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
    allContent.clear();

    try
    {
        if (recursive)
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath))
            {
                allContent.push_back(entry.path().generic_string());
            }
        }
        else
        {
            for (const auto& entry : std::filesystem::directory_iterator(directoryPath))
            {
                allContent.push_back(entry.path().generic_string());
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        LOG("Error listing directory: %s", e.what());
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