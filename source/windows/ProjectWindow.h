#pragma once
#include "UIWindow.h"
#include "../EventListener.h"
#include "imgui.h"
#include <string>
#include <vector>

struct DirectoryNode
{
    std::string name;
    std::string path;
    std::string extension;
    bool isDirectory;

    std::vector<DirectoryNode*> children;
    DirectoryNode* parent = nullptr;

    DirectoryNode(std::string _name, std::string _path, bool _isDir)
        : name(_name), path(_path), isDirectory(_isDir) {
    }

    ~DirectoryNode() {
        for (auto child : children) delete child;
        children.clear();
    }
};

class ProjectWindow : public UIWindow, public EventListener
{
public:
    ProjectWindow(bool active);
    virtual ~ProjectWindow();

    void Awake() override;
    void CleanUp() override;

    void Draw() override;
    void DrawFolderTree();
    void DrawTreeNodeRecursive(DirectoryNode* node);
    void DrawFolderContent();

private:
    void RefreshTree();
    void BuildTreeRecursive(const std::string& path, DirectoryNode* parent);

    void ChangeCurrentNode(DirectoryNode* direcoryNode);
    DirectoryNode* FindNodeByPath(DirectoryNode* root, const std::string& path);
    void SelectNode(DirectoryNode* direcoryNode);

    void AddAssetToScene(DirectoryNode* direcoryNode);

    unsigned int GetIconTextureWithExtension(const std::string& extension);
   
    void OnEvent(const Event& event);

private:

    std::string rootPath;

    DirectoryNode* rootNode = nullptr;
    DirectoryNode* currentNode = nullptr;
    std::vector<DirectoryNode*> selectedNodes = {};
    std::vector<DirectoryNode*> nodesToDelete = {};

    std::string pathToDrop;

    unsigned int folderIconTextureID = 0;
    unsigned int fileIconTextureID = 0;
    unsigned int modelIconTextureID = 0;
    unsigned int imageIconTextureID = 0;
    unsigned int scriptIconTextureID = 0;
    unsigned int sceneIconTextureID = 0;

    bool expandTreeToSelection = false;
    bool dragging = false;
    bool isFocused = false;
};