#include "FileUtils.h"
#include "Log.h"

std::string GetDirectoryFromPath(const std::string& filePath)
{
    std::filesystem::path path(filePath);
    return path.parent_path().string();
}

std::string GetFileExtension(const std::string& filePath)
{
    std::filesystem::path path(filePath);
    std::string ext = path.extension().string();

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
    return path.filename().string();
}

std::string FindFileInDirectory(const std::string& directoryPath, const std::string& fileName)
{
    try
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath))
        {
            if (entry.is_regular_file() && entry.path().filename() == fileName)
            {
                return entry.path().string();
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

    try
    {
        if (recursive)
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath))
            {
                allContent.push_back(entry.path().string());
            }
        }
        else
        {
            for (const auto& entry : std::filesystem::directory_iterator(directoryPath))
            {
                allContent.push_back(entry.path().string());
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