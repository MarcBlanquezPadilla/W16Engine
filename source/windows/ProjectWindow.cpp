#include "ProjectWindow.h"
#include "../ModuleLoader.h"
#include "../ModuleEvents.h"
#include "../Engine.h"
#include "../utils/Log.h"
#include "../utils/FileUtils.h"

#include "imgui.h"

ProjectWindow::ProjectWindow(bool active) : UIWindow("Project", active)
{
    
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
}

void ProjectWindow::CleanUp()
{
    Engine::GetInstance().moduleEvents->UnsubscribeAll(this);
}

void ProjectWindow::Draw()
{
    if (!is_active) return;

    if (!ImGui::Begin(name, &is_active))
    {
        ImGui::End();
        return;
    }

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
    ImGui::Separator();

    DrawFolderContent();
    ImGui::EndChild();

    ImGui::Columns(1);
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
            p = p->parent; // Subimos un nivel
        }
    }

    bool isOpen = ImGui::TreeNodeEx(node->name.c_str(), flags);

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

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

        if (ImGui::ImageButton("##icon", iconTexture, ImVec2(thumbnailSize, thumbnailSize)))
        {
            selectedNode = child;
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            if (IsFileDirectory(child->path))
            {
                ChangeCurrentNode(child);
            }
            else
            {
                Engine::GetInstance().moduleLoader->LoadModel(child->path);
            }
        }
        ImGui::PopStyleColor();

        float textWidth = ImGui::CalcTextSize(GetFileName(child->name).c_str()).x;

        if (textWidth < thumbnailSize)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (thumbnailSize - textWidth) * 0.5f);
        }

        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + thumbnailSize);
        ImGui::Text(child->name.c_str());
        ImGui::PopTextWrapPos();

        ImGui::EndGroup();

        ImGui::PopID();

        ImGui::NextColumn();
    }

    ImGui::Columns(1);
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
    if (rootNode)
    {
        delete rootNode;
        rootNode = nullptr;
    }

    rootNode = new DirectoryNode(rootPath, rootPath, true);
    currentNode = rootNode;

    BuildTreeRecursive(rootPath, rootNode);
}

void ProjectWindow::BuildTreeRecursive(const std::string& path, DirectoryNode* parentNode)
{
    for (const auto& entry : GetListDirectoryContents(path))
    {
        std::string entryPath = entry;
        std::string entryName = GetFileName(entry);
        std::string extension = GetFileExtension(entry);
        bool isDir = IsFileDirectory(entry);

        if (GetFileExtension(entry) == "meta") continue;

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

    default:
        break;
    }
}