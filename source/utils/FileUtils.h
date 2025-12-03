#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include "../Global.h"

std::string GetDirectoryFromPath(const std::string& filePath);

std::string GetFileExtension(const std::string& filePath);

std::string GetFileName(const std::string& filePath);

std::string FindFileInDirectory(const std::string& directoryPath, const std::string& fileName);

std::string GetPreviousPath(const std::string& directoryPath);

std::vector<std::string> GetListDirectoryContents(const std::string& directoryPath, bool recursive = false);

bool IsFileDirectory(const std::string& directoryPath);

bool DoesFileExist(const std::string& filePath);

bool DoesFileHasMeta(const std::string& directoryPath);

std::string GetMetaPath(const std::string& directoryPath);

std::string GetLibraryPath(const UID uid);

bool CreateDirectory(const std::string& directoryPath);

int64_t GetLastModificationTime(const std::string& path);