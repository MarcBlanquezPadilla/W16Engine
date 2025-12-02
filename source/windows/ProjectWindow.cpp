#include "ProjectWindow.h"
#include "../ModuleLoader.h"
#include "../Engine.h"
#include "../utils/Log.h"
#include "../utils/FileUtils.h"

#include "imgui.h"

ProjectWindow::ProjectWindow(bool active) : UIWindow("Project", active)
{
    currentPath = "Assets";
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
    DrawFolderTree("Assets");
    ImGui::EndChild();

    //FOLDER CONTENT
    ImGui::NextColumn();
    ImGui::BeginChild("FolderContent", ImVec2(0, 0), true);
    if (ImGui::Button("Back"))
    {
        currentPath = GetPreviousPath(currentPath);
        updateScrollToSelection = true;
    }
    ImGui::SameLine();
    ImGui::Text("| Current: %s", currentPath.c_str());
    ImGui::Separator();

    DrawFolderContent();
    ImGui::EndChild();

    ImGui::Columns(1);
    ImGui::End();
}

void ProjectWindow::DrawFolderTreeRecursive(const std::string& path)
{
    std::vector<std::string> contents = GetListDirectoryContents(path);

    for (const std::string& itemPath : contents)
    {
        if (!IsFileDirectory(itemPath)) continue;

        std::string folderName = GetFileName(itemPath);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

        if (itemPath == currentPath)
            flags |= ImGuiTreeNodeFlags_Selected;

        bool hasChilds = false;

        for (const std::string& subItem : GetListDirectoryContents(itemPath)) {
            if (IsFileDirectory(subItem)) { hasChilds = true; break; }
        }
        if (!hasChilds) flags |= ImGuiTreeNodeFlags_Leaf;

        if (currentPath.find(itemPath) == 0 && itemPath.size() < currentPath.size())
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
        }

        bool nodeOpen = ImGui::TreeNodeEx(folderName.c_str(), flags);

        if (itemPath == currentPath && updateScrollToSelection)
        {
            ImGui::SetScrollHereY();
            updateScrollToSelection = false;
        }

        if (ImGui::IsItemClicked())
        {
            currentPath = itemPath; 
        }

        if (nodeOpen)
        {
            DrawFolderTreeRecursive(itemPath);
            ImGui::TreePop();
        }
    }
}

void ProjectWindow::DrawFolderTree(const std::string& rootPath)
{
    std::string rootName = GetFileName(rootPath);
    ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_DefaultOpen;

    if (rootPath == currentPath)
        rootFlags |= ImGuiTreeNodeFlags_Selected;

    bool rootOpen = ImGui::TreeNodeEx(rootName.c_str(), rootFlags);

    if (ImGui::IsItemClicked())
    {
        currentPath = rootPath;
    }

    if (rootOpen)
    {
        DrawFolderTreeRecursive(rootPath);
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
    for (const std::string& path : GetListDirectoryContents(currentPath))
    {
        ImGui::PushID(i++);
        
        ImGui::BeginGroup();
        ImTextureID iconTexture = GetIconTextureWithExtension(GetFileExtension(path));

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

        if (ImGui::ImageButton("##icon", iconTexture, ImVec2(thumbnailSize, thumbnailSize)))
        {
            selectedPath = path;
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            if (IsFileDirectory(path))
            {
                currentPath = path;
                updateScrollToSelection;
            }
        }
        ImGui::PopStyleColor();

        float textWidth = ImGui::CalcTextSize(GetFileName(path).c_str()).x;

        if (textWidth < thumbnailSize)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (thumbnailSize - textWidth) * 0.5f);
        }

        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + thumbnailSize);
        ImGui::Text(GetFileName(path).c_str());
        ImGui::PopTextWrapPos();

        ImGui::EndGroup();

        ImGui::PopID();

        ImGui::NextColumn();
    }

    ImGui::Columns(1);
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