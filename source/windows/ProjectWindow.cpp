#include "ProjectWindow.h"
#include "../ModuleLoader.h"
#include "../ModuleEvents.h"
#include "../ModuleInput.h"
#include "../ModuleResources.h"
#include "../ModuleEditor.h"
#include "../Engine.h"
#include "../Global.h"
#include "../utils/Log.h"
#include "../utils/Config.h"
#include "../utils/FileUtils.h"
#include "../resources/ResourceTexture.h"

#include "imgui.h"

ProjectWindow::ProjectWindow(bool active) : UIWindow("Project", active)
{
    currentNode = nullptr;
}

ProjectWindow::~ProjectWindow()
{
    
}



void ProjectWindow::Awake()
{
    fileIcon = (ResourceTexture*)Engine::GetInstance().moduleResources->RequestResource(ICON_FILE);
    folderIcon = (ResourceTexture*)Engine::GetInstance().moduleResources->RequestResource(ICON_FOLDER);
    modelIcon = (ResourceTexture*)Engine::GetInstance().moduleResources->RequestResource(ICON_MODEL);
    meshIcon = (ResourceTexture*)Engine::GetInstance().moduleResources->RequestResource(ICON_MESH);
    animIcon = (ResourceTexture*)Engine::GetInstance().moduleResources->RequestResource(ICON_ANIMATION);
    imageIcon = (ResourceTexture*)Engine::GetInstance().moduleResources->RequestResource(ICON_IMAGE);
    scriptIcon = (ResourceTexture*)Engine::GetInstance().moduleResources->RequestResource(ICON_SCRIPT);
    sceneIcon = (ResourceTexture*)Engine::GetInstance().moduleResources->RequestResource(ICON_SCENE);

    rootPath = "Assets";
    RefreshTree();

    //EVENTS
    Engine::GetInstance().moduleEvents->Subscribe(Event::Type::AssetsChanged, this);
    Engine::GetInstance().moduleEvents->Subscribe(Event::Type::FileDropped, this);
}

void ProjectWindow::CleanUp()
{
    if (rootNode != nullptr)
    {
        delete rootNode;
        rootNode = nullptr;
    }
    Engine::GetInstance().moduleEvents->UnsubscribeAll(this);
}

void ProjectWindow::Draw()
{
    isFocused = false;
    windowChanged = false;
    nodesToDelete.clear();

    if (!is_active) return;

    if (!ImGui::Begin(name, &is_active))
    {
        ImGui::End();
        return;
    }

    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        ImGui::SetWindowFocus();
    }

    isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

    if (currentAsset)
    {
        ImGui::BeginChild("Subresources", ImVec2(0, 0), true);
        DrawSubresources();
        ImGui::EndChild();
    }
    else
    {
        ImGui::Columns(2, "ProjectColumns", true);

        //FOLDERS
        ImGui::BeginChild("FolderTree", ImVec2(0, 0), true);
        DrawFolderTree();
        ImGui::EndChild();

        //FOLDER CONTENT
        ImGui::NextColumn();
        ImGui::BeginChild("FolderContent", ImVec2(0, 0), true);
        if (ImGui::Button("Back") && currentNode->parent)
        {
            currentNode = currentNode->parent;
        }
        ImGui::SameLine();
        ImGui::Text("| Current: %s", currentNode->path.c_str());

        DrawFolderContent();
        ImGui::EndChild();

        ImGui::Columns(1);
    }

    

    //DELETE
    if (!ImGui::IsMouseDragging(0) && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && selectedNodes.size() > 0 && Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_DELETE) == KEY_DOWN)
    {
        nodesToDelete = selectedNodes;
    }

    if (!nodesToDelete.empty())
    {
        for (DirectoryNode* node : nodesToDelete)
        {
            std::string pathToDelete = node->path;

            UID uid = Engine::GetInstance().moduleResources->Find(pathToDelete);

            if (DeleteAsset(pathToDelete))
            {
                Engine::GetInstance().moduleResources->RemoveResource(uid);
                if (currentNode == node)
                {
                    currentNode = rootNode;
                }
                windowChanged = true;
            }

            
        }

        selectedNodes.clear();
        nodesToDelete.clear();
        currentAsset = nullptr;
       
    }

    ImGui::End();

    if (windowChanged)
    {
        Engine::GetInstance().moduleResources->PublishAssetChangedEvent();
    }
}


void ProjectWindow::DrawFolderTree()
{
    if (rootNode)
    {
        DrawTreeNodeRecursive(rootNode);
    }
    expandTreeToSelection = false;
}

void ProjectWindow::DrawTreeNodeRecursive(DirectoryNode* node)
{
    if (!node->isDirectory) return;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

    if (node == currentNode)
        flags |= ImGuiTreeNodeFlags_Selected;

    if (node->children.empty())
    {
        bool hasSubDirs = false;
        for (auto c : node->children) if (c->isDirectory) hasSubDirs = true;

        if (!hasSubDirs) flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (expandTreeToSelection)
    {
        DirectoryNode* p = currentNode;
        while (p != nullptr)
        {
            if (p == node)
            {
                ImGui::SetNextItemOpen(true, ImGuiCond_Always);
                break;
            }
            p = p->parent;
        }
    }

    bool isOpen = ImGui::TreeNodeEx(node->name.c_str(), flags);

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(ASSETS_DRAG))
        {
            const char* dataPtr = (const char*)payload->Data;
            const char* endPtr = dataPtr + payload->DataSize;

            while (dataPtr < endPtr)
            {
                std::string path = dataPtr;
                if (path.empty()) break;

                if (path != node->path)
                {
                    bool isMovingParentIntoChild = (node->path.find(path) != std::string::npos);

                    if (!isMovingParentIntoChild)
                    {
                        MoveAssetToFolder(path, node->path);
                        std::string metaPath = path + ".meta";
                        if (DoesFileExist(metaPath))
                            MoveAssetToFolder(metaPath, node->path);
   
                        

                        LOG(LogType::LOG_INFO, "Movido %s a %s (Tree View)", path.c_str(), node->path.c_str());
                    }
                }
                dataPtr += path.length() + 1;
            }
            windowChanged = true;
        }
        ImGui::EndDragDropTarget();
    }

    if (ImGui::IsItemClicked())
    {
        ChangeCurrentNode(node);
    }

    if (isOpen)
    {
        for (DirectoryNode* child : node->children)
        {
            if (child->isDirectory)
                DrawTreeNodeRecursive(child);
        }
        ImGui::TreePop();
    }
}

void ProjectWindow::DrawFolderContent() 
{
    if (currentNode == nullptr)
    {
        currentNode = rootNode;
    }

    float padding = 16.0f;
    float thumbnailSize = 64.0f;
    float cellSize = thumbnailSize + padding;

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1) columnCount = 1;

    ImGui::Columns(columnCount, 0, false);

    int i = 0;
    for (DirectoryNode* child : currentNode->children)
    {
        ImGui::PushID(i++);
        
        ImGui::BeginGroup();
        ImTextureID iconTexture = GetIconTextureWithResource(child->recourse);

        bool isSelected = std::find(selectedNodes.begin(), selectedNodes.end(), child) != selectedNodes.end();

        if (isSelected) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
        ImGui::ImageButton("##icon", iconTexture, ImVec2(thumbnailSize, thumbnailSize), ImVec2(0, 1), ImVec2(1, 0));
        if (isSelected) ImGui::PopStyleColor();

        if (ImGui::BeginDragDropSource())
        {
            if (!isSelected) {
                selectedNodes.clear();
                selectedNodes.push_back(child);
            }

            std::string payloadBuffer;
            for (DirectoryNode* node : selectedNodes)
            {
                payloadBuffer += node->path;
                payloadBuffer += '\0';
            }

            ImGui::SetDragDropPayload(ASSETS_DRAG, payloadBuffer.c_str(), payloadBuffer.size());
            if (selectedNodes.size() == 1)
                ImGui::Text("Moviendo %s", child->name.c_str());
            else
                ImGui::Text("Moviendo %d archivos", selectedNodes.size());

            ImGui::EndDragDropSource();
        }

        if (child->isDirectory)
        {
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(ASSETS_DRAG))
                {
                    const char* dataPtr = (const char*)payload->Data;
                    const char* endPtr = dataPtr + payload->DataSize;

                    while (dataPtr < endPtr)
                    {
                        std::string path = dataPtr;
                        if (path.empty()) break;

                        std::string fileName = GetFileName(path);
                        std::string destination = child->path + "/" + fileName;

                        MoveAssetToFolder(path, child->path);

                        std::string metaPath = path + ".meta";
                        if (DoesFileExist(metaPath)) MoveAssetToFolder(metaPath, child->path);

                        LOG(LogType::LOG_INFO, "Movido %s a %s", path.c_str(), child->path.c_str());

                        dataPtr += path.length() + 1;
                    }
                    windowChanged = true;
                }
                ImGui::EndDragDropTarget();
            }
        }


        bool ctrl = Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT;
        bool shift = Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT;
        bool erase = !(ctrl || shift);

        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
        {
            if (!isSelected)
            {
               SelectNode(child, erase);
            }
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0))
        {
            if (!ImGui::IsMouseDragging(0) && !ctrl && isSelected)
            {
                SelectNode(child, erase);
            }
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            if (child->isDirectory) { selectedNodes.clear(); ChangeCurrentNode(child); }
            else {
                if (!child->subRecourses.empty())
                    currentAsset = child;
            }
        }


        if (ImGui::BeginPopupContextItem("ItemContext"))
        {
            bool isSelected = std::find(selectedNodes.begin(), selectedNodes.end(), child) != selectedNodes.end();
            if (!isSelected)
            {
                SelectNode(child, false);
            }

            if (ImGui::MenuItem("Delete")) 
            {
                nodesToDelete = selectedNodes;
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Rename"))
            {
                renamingNode = child;

                std::string nameWithoutExt = GetFileNameNoExtension(child->name);
                strcpy_s(renameBuffer, sizeof(renameBuffer), nameWithoutExt.c_str());

                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        if (renamingNode == child)
        {
            ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;

            ImGui::SetKeyboardFocusHere();

            ImGui::PushItemWidth(thumbnailSize);
            if (ImGui::InputText("##rename", renameBuffer, IM_ARRAYSIZE(renameBuffer), flags))
            {
                windowChanged = ExecuteRename(child, std::string(renameBuffer));
                renamingNode = nullptr;
            }
            ImGui::PopItemWidth();

            if (!ImGui::IsItemActive() && (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)))
            {
                renamingNode = nullptr;
            }
        }
        else
        {
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + thumbnailSize);
            ImGui::Text(child->name.c_str());
            ImGui::PopTextWrapPos();
        }

        ImGui::EndGroup();

        ImGui::PopID();

        ImGui::NextColumn();
    }

    ImGui::Columns(1);

    if (ImGui::BeginPopupContextWindow("BackgroundContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
    {
        if (ImGui::MenuItem("Refresh"))
        {
            Engine::GetInstance().moduleResources->CheckForFilesModifications();
        }
        
        ImGui::Separator();

        if (ImGui::BeginMenu("Create"))
        {
            if (ImGui::MenuItem("Folder")) {

                CreateDirectory(currentNode->path + "/NewFolder");
                Engine::GetInstance().moduleResources->PublishAssetChangedEvent();
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Material")) { }
            if (ImGui::MenuItem("Script")) { }

            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }
}

void ProjectWindow::DrawSubresources()
{
    if (ImGui::Button("Back"))
    {
        currentAsset = nullptr;
    }

    if (currentAsset == nullptr)
    {
        return;
    }

    ImGui::SameLine();
    ImGui::Text("| Current file: %s", currentAsset->name.c_str());
    ImGui::Separator();

    float padding = 16.0f;
    float thumbnailSize = 64.0f;
    float cellSize = thumbnailSize + padding;

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1) columnCount = 1;

    ImGui::Columns(columnCount, 0, false);

    int i = 0;
    for (const Resource* resource : currentAsset->subRecourses)
    {
        ImGui::PushID(i++);

        ImGui::BeginGroup();
        ImTextureID iconTexture = GetIconTextureWithResource(resource);
        ImGui::ImageButton("##icon", iconTexture, ImVec2(thumbnailSize, thumbnailSize), ImVec2(0, 1), ImVec2(1, 0));

        if (ImGui::BeginDragDropSource())
        {
            UID resourceUID = resource->GetUID();
            ImGui::SetDragDropPayload(RESOURCE_DRAG, &resourceUID, sizeof(UID));
            ImGui::Text("Moviendo %s", resource->GetName());

            ImGui::EndDragDropSource();
        }

        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + thumbnailSize);
        const char* name = resource->GetName();
        ImGui::Text(name);
        ImGui::PopTextWrapPos();

        ImGui::EndGroup();

        ImGui::PopID();

        ImGui::NextColumn();
    }
}

void ProjectWindow::ChangeCurrentNode(DirectoryNode* directoryNode)
{
    currentNode = directoryNode;
    expandTreeToSelection = true;
}

unsigned int ProjectWindow::GetIconTextureWithResource(const Resource* resource)
{
    if (resource)
    {
        switch (resource->GetType())
        {
        case Resource::Type::texture:
            return imageIcon->gpuID;
            break;
        case Resource::Type::model:
            return modelIcon->gpuID;
            break;
        case Resource::Type::scene:
            return sceneIcon->gpuID;
            break;
        case Resource::Type::mesh:
            return meshIcon->gpuID;
            break;
        case Resource::Type::animation:
            return animIcon->gpuID;
            break;
        default:
            return fileIcon->gpuID;
            break;
        }
    }
    else return folderIcon->gpuID;

}

void ProjectWindow::RefreshTree()
{
    std::string previousPath = "Assets";
    if (currentNode != nullptr) previousPath = currentNode->path;

    std::string previousAssetPath = "";
    if (currentAsset != nullptr) previousAssetPath = currentAsset->path;

    if (rootNode)
    {
        delete rootNode;
        rootNode = nullptr;
    }

    currentAsset = nullptr;

    rootNode = new DirectoryNode(rootPath, rootPath, true);
    rootNode->recourse = nullptr;
    rootNode->subRecourses = {};
    BuildTreeRecursive(rootPath, rootNode);

    DirectoryNode* nodeToRestore = FindNodeByPath(rootNode, previousPath);

    if (nodeToRestore != nullptr)
    {
        currentNode = nodeToRestore;
    }
    else
    {
        currentNode = rootNode;
    }

    if (!previousAssetPath.empty())
    {
        DirectoryNode* assetNode = FindNodeByPath(rootNode, previousAssetPath);
        if (assetNode)
        {
            currentAsset = assetNode;
        }
    }

    expandTreeToSelection = true;
}

void ProjectWindow::BuildTreeRecursive(const std::string& path, DirectoryNode* parentNode)
{
    for (const auto& entry : GetListDirectoryContents(path))
    {
        std::string entryPath = entry;
        std::string entryName = GetFileName(entry);
        std::string extension = GetFileExtension(entry);
        bool isDir = IsFileDirectory(entry);

        DirectoryNode* newNode = new DirectoryNode(entryName, entryPath, isDir);
        newNode->parent = parentNode;
        newNode->extension = extension;
        newNode->recourse = nullptr;
        newNode->subRecourses = {};

        bool succes = true;

        if (!isDir)
        {
            if (DoesFileHasMeta(entry)) 
            {
                Config meta;
                std::string metaPath = GetMetaPath(entry);
                if (meta.Load(metaPath.c_str()))
                {
                    UID resourceUID = meta.GetUInt("UID");
                    if (resourceUID != 0 && Engine::GetInstance().moduleResources->PeekResource(resourceUID))
                    {
                        newNode->recourse = Engine::GetInstance().moduleResources->PeekResource(resourceUID);
                        unsigned int subResourceNum = meta.GetUInt("ReferedObjects");

                        if (subResourceNum > 0)
                        {

                            Config refNode = meta.GetChild("ReferedObject");

                            while (refNode.IsValid())
                            {
                                UID childUID = (UID)refNode.GetUInt("UID");
                                if (childUID != 0 && Engine::GetInstance().moduleResources->PeekResource(childUID))
                                {
                                    newNode->subRecourses.push_back(Engine::GetInstance().moduleResources->PeekResource(childUID));
                                }
                                refNode = refNode.GetNextSibling("ReferedObject");
                            }
                        }
                    }
                    else succes = false;
                }
            }
            else succes = false;
        }

        if (succes)
        {
            parentNode->children.push_back(newNode);

            if (isDir)
            {
                BuildTreeRecursive(entryPath, newNode);
            }
        }
        else
        {
            delete newNode;
            newNode = nullptr;
        }
    }
}

DirectoryNode* ProjectWindow::FindNodeByPath(DirectoryNode* node, const std::string& path)
{
    if (node->path == path) return node;

    for (DirectoryNode* child : node->children)
    {
        if (child->isDirectory)
        {
            DirectoryNode* result = FindNodeByPath(child, path);
            if (result != nullptr) return result;
        }
    }
    return nullptr;
}

void ProjectWindow::SelectNode(DirectoryNode* direcoryNode, bool eraseSelecteds = true)
{
    if (eraseSelecteds) selectedNodes.clear();
    if (std::find(selectedNodes.begin(), selectedNodes.end(), direcoryNode) == selectedNodes.end())
    {
        selectedNodes.push_back(direcoryNode);
    }
}

bool ProjectWindow::ExecuteRename(DirectoryNode* node, const std::string& newName)
{
    std::string directory = GetPreviousPath(node->path);
    std::string extension = node->extension.empty() ? "" : "." + node->extension;

    std::string oldPath = node->path;
    std::string oldMetaPath = oldPath + ".meta";

    std::string newPath = directory + "/" + newName + extension;
    std::string newMetaPath = newPath + ".meta";

    if (DoesFileExist(newPath))
    {
        LOG(LogType::LOG_ERROR, "A file with that name already exists.");
        return false;
    }

    if (std::rename(oldPath.c_str(), newPath.c_str()) == 0)
    {
        if (DoesFileExist(oldMetaPath)) std::rename(oldMetaPath.c_str(), newMetaPath.c_str());


        if (node->isDirectory)
        {
            Engine::GetInstance().moduleResources->MoveFolder(oldPath, newPath);
        }
        else
        {
            Engine::GetInstance().moduleResources->MoveResource(oldPath, newPath);
        }

        LOG(LogType::LOG_INFO, "Renamed %s to %s", oldPath.c_str(), newPath.c_str());
        return true;
    }
    else
    {
        LOG(LogType::LOG_ERROR, "Failed renaming file from %s to %s", oldPath.c_str(), newPath.c_str());
        return false;
    }
}

void ProjectWindow::OnEvent(const Event& event)
{
    switch (event.type)
    {
    case Event::Type::AssetsChanged:
    {
        {
            RefreshTree();
        }
        break;
    }

    case Event::Type::FileDropped:
    {
        {
            std::string endPath = DoesFileExist(currentNode->path) ? currentNode->path : rootPath;
            CopyAssetToFolder(event.data.string.string, currentNode->path);
        }
        break;
    }

    default:
        break;
    }
}
