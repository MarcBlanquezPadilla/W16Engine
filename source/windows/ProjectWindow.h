#pragma once
#include "UIWindow.h"
#include <string>

class ProjectWindow : public UIWindow
{
public:
    ProjectWindow(bool active);
    virtual ~ProjectWindow();

    void Draw() override;
    void DrawFolderTree(const std::string& rootPath);
    void DrawFolderTreeRecursive(const std::string& path);

private:

    std::string currentPath;
};