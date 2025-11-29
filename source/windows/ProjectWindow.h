#pragma once
#include "UIWindow.h"
#include "imgui.h"
#include <string>

class ProjectWindow : public UIWindow
{
public:
    ProjectWindow(bool active);
    virtual ~ProjectWindow();

    void Awake();

    void Draw() override;
    void DrawFolderTree(const std::string& rootPath);
    void DrawFolderTreeRecursive(const std::string& path);
    void DrawFolderContent();

private:
    unsigned int GetIconTextureWithExtension(const std::string& extension);

private:

    std::string currentPath;
    std::string selectedPath;

    unsigned int folderIconTextureID = 0;
    unsigned int fileIconTextureID = 0;
    unsigned int modelIconTextureID = 0;
    unsigned int imageIconTextureID = 0;
    unsigned int scriptIconTextureID = 0;
    unsigned int sceneIconTextureID = 0;

    bool updateScrollToSelection = false;
};