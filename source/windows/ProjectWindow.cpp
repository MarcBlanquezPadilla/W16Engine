#include "ProjectWindow.h"
#include "imgui.h"
#include "../utils/Log.h"
#include "../utils/FileUtils.h"

ProjectWindow::ProjectWindow(bool active) : UIWindow("Project", active)
{
    currentPath = "Assets";
}

ProjectWindow::~ProjectWindow()
{
    
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

    ImGui::BeginChild("FolderTree", ImVec2(0, 0), true);
    DrawFolderTree("Assets");
    ImGui::EndChild();

    ImGui::NextColumn();
    ImGui::BeginChild("FolderContent", ImVec2(0, 0), true);
    ImGui::Text("Current: %s", currentPath.c_str());
    ImGui::Separator();

    for (const std::string& path : GetListDirectoryContents(currentPath))
    {
        std::string fileName = GetFileName(path);
        bool isDirectory = IsFileDirectory(path);

        if (isDirectory)
            ImGui::Text("[DIRE] ");
        else
            ImGui::Text("[FILE] ");

        ImGui::SameLine();

        if (ImGui::Selectable(fileName.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
        {
            if (isDirectory && ImGui::IsMouseDoubleClicked(0))
            {
                currentPath = path;
            }
        }
    }
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
        for (const std::string& subItem : GetListDirectoryContents(itemPath))
        {
            if (IsFileDirectory(subItem))
            {
                hasChilds = true;
                break;
            }
        }

        if (!hasChilds)
            flags |= ImGuiTreeNodeFlags_Leaf;

        bool nodeOpen = ImGui::TreeNodeEx(folderName.c_str(), flags);

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