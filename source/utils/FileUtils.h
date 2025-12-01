#pragma once
#include <string>
#include <vector>
#include <filesystem>

std::string GetDirectoryFromPath(const std::string& filePath);

std::string GetFileExtension(const std::string& filePath);

std::string GetFileName(const std::string& filePath);

std::string FindFileInDirectory(const std::string& directoryPath, const std::string& fileName);

std::string GetPreviousPath(const std::string& directoryPath);

std::vector<std::string> GetListDirectoryContents(const std::string& directoryPath, bool recursive = false);

bool IsFileDirectory(const std::string& directoryPath);

bool DoesFileExist(const std::string& filePath);

bool CreateDirectory(const std::string& directoryPath);