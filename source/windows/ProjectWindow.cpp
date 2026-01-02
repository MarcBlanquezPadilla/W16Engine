#include "ProjectWindow.h"
#include "../ModuleLoader.h"
#include "../ModuleEvents.h"
#include "../ModuleInput.h"
#include "../ModuleResources.h"
#include "../ModuleEditor.h"
#include "../Engine.h"
#include "../Global.h"
#include "../utils/Log.h"
#include "../utils/FileUtils.h"

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
    int width = 0, height = 0;
    Engine::GetInstance().moduleLoader->LoadTexture("Resources/folder.png", folderIconTextureID, width, height, true);
    
    width = 0, height = 0;
    Engine::GetInstance().moduleLoader->LoadTexture("Resources/file.png", fileIconTextureID, width, height, true);

    width = 0, height = 0;
    Engine::GetInstance().moduleLoader->LoadTexture("Resources/model.png", modelIconTextureID, width, height, true);

    width = 0, height = 0;
    Engine::GetInstance().moduleLoader->LoadTexture("Resources/image.png", imageIconTextureID, width, height, true);

    width = 0, height = 0;
    Engine::GetInstance().moduleLoader->LoadTexture("Resources/scene.png", sceneIconTextureID, width, height, true);

    width = 0, height = 0;
    Engine::GetInstance().moduleLoader->LoadTexture("Resources/script.png", scriptIconTextureID, width, height, true);

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
                windowChanged = true;
            }
        }

        selectedNodes.clear();
        nodesToDelete.clear();
    }

    if (windowChanged)
    {
        Engine::GetInstance().moduleResources->PublishAssetChangedEvent();
    }

    ImGui::End();
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

                if (path != node->path && path.find(node->path) == std::string::npos)
                {
                    MoveAssetToFolder(path, node->path);
                    std::string metaPath = path + ".meta";
                    if (DoesFileExist(metaPath)) MoveAssetToFolder(metaPath, node->path);
                    LOG("Movido %s a %s (Tree View)", path.c_str(), node->path.c_str());
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
        ImTextureID iconTexture = GetIconTextureWithExtension(child->extension);

        bool isSelected = std::find(selectedNodes.begin(), selectedNodes.end(), child) != selectedNodes.end();

        if (isSelected) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
        ImGui::ImageButton("##icon", iconTexture, ImVec2(thumbnailSize, thumbnailSize));
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

                        LOG("Movido %s a %s", path.c_str(), child->path.c_str());

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
                Resource::Type type = Engine::GetInstance().moduleResources->GetTypeFromExtension(child->path);
                if (type == Resource::model) Engine::GetInstance().moduleLoader->LoadModel(child->path);
                else if (type == Resource::scene) Engine::GetInstance().moduleLoader->CleanAndLoadScene(child->path);
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

void ProjectWindow::ChangeCurrentNode(DirectoryNode* directoryNode)
{
    currentNode = directoryNode;
    expandTreeToSelection = true;
}

unsigned int ProjectWindow::GetIconTextureWithExtension(const std::string& extension)
{
    if (extension.empty()) return folderIconTextureID;

    if (extension == "fbx" ||
        extension == "obj" ||
        extension == "gltf" ||
        extension == "glb" ||
        extension == "dae" ||
        extension == "blend")
    {
        return modelIconTextureID;
    }

    if (extension == "png" ||
        extension == "jpg" ||
        extension == "jpeg" ||
        extension == "tga" ||
        extension == "bmp" ||
        extension == "dds" ||
        extension == "tiff")
    {

        return imageIconTextureID;
    }

    if (extension == "cpp" ||
        extension == "h" ||
        extension == "hpp" ||
        extension == "cs" ||
        extension == "lua" ||
        extension == "py" ||
        extension == "json" ||
        extension == "xml")
    {
        return scriptIconTextureID;
    }

    if (extension == "wscene")
    {
        return sceneIconTextureID;
    }


    return fileIconTextureID;
}

void ProjectWindow::RefreshTree()
{
    std::string previousPath = "Assets";
    if (currentNode != nullptr)
    {
        previousPath = currentNode->path;
    }

    if (rootNode)
    {
        delete rootNode;
        rootNode = nullptr;
    }

    rootNode = new DirectoryNode(rootPath, rootPath, true);
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

        if (!IsFileDirectory(entry) && Engine::GetInstance().moduleResources->GetTypeFromExtension(entry) == Resource::Type::unknown) continue;

        DirectoryNode* newNode = new DirectoryNode(entryName, entryPath, isDir);
        newNode->parent = parentNode;
        newNode->extension = extension;
        parentNode->children.push_back(newNode);

        if (isDir)
        {
            BuildTreeRecursive(entryPath, newNode);
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
        LOG("Error: A file with that name already exists.");
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

        LOG("Renamed %s to %s", oldPath.c_str(), newPath.c_str());
        return true;
    }
    else
    {
        LOG("Error renaming file from %s to %s", oldPath.c_str(), newPath.c_str());
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
            MoveAssetToFolder(event.data.string.filePath, currentNode->path);
        }
        break;
    }

    default:
        break;
    }
}
