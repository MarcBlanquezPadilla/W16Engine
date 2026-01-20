#pragma once
#include "UIWindow.h"
#include "../EventListener.h"
#include "imgui.h"
#include <string>
#include <vector>

class Resource;
class ResourceTexture;

struct DirectoryNode
{
    std::string name;
    std::string path;
    std::string extension;
    bool isDirectory;

    const Resource* recourse;
    std::vector<const Resource*> subRecourses;
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
    void DrawSubresources();

private:
    void RefreshTree();
    void BuildTreeRecursive(const std::string& path, DirectoryNode* parent);

    void ChangeCurrentNode(DirectoryNode* direcoryNode);
    DirectoryNode* FindNodeByPath(DirectoryNode* root, const std::string& path);
    void SelectNode(DirectoryNode* direcoryNode, bool eraseSelecteds);

    void AddAssetToScene(DirectoryNode* direcoryNode);

    unsigned int GetIconTextureWithExtension(const std::string& extension);
    unsigned int GetIconTextureWithResource(const Resource* resource);
   
    bool ExecuteRename(DirectoryNode* node, const std::string& newName);

    void OnEvent(const Event& event);

private:

    std::string rootPath;

    DirectoryNode* rootNode = nullptr;
    DirectoryNode* currentNode = nullptr;
    DirectoryNode* currentAsset = nullptr;
    std::vector<DirectoryNode*> selectedNodes = {};
    std::vector<DirectoryNode*> nodesToDelete = {};

    ResourceTexture* folderIcon = 0;
    ResourceTexture* fileIcon = 0;
    ResourceTexture* modelIcon = 0;
    ResourceTexture* meshIcon = 0;
    ResourceTexture* animIcon = 0;
    ResourceTexture* imageIcon = 0;
    ResourceTexture* scriptIcon = 0;
    ResourceTexture* sceneIcon = 0;

    bool expandTreeToSelection = false;
    bool isFocused = false;
    bool windowChanged = false;

private:
    DirectoryNode* renamingNode = nullptr;
    char renameBuffer[128];
    char searchBuffer[64];
    std::string query;
    bool searching = false;
};