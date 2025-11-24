#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

std::string GetDirectoryFromPath(const std::string& filePath);

std::string GetFileExtension(const std::string& filePath);

std::string GetFileName(const std::string& filePath);

std::string FindFileInDirectory(const std::string& directoryPath, const std::string& fileName);

std::vector<std::string> GetListDirectoryContents(const std::string& directoryPath);