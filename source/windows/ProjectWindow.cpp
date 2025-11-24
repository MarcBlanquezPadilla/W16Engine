#include "ProjectWindow.h"
#include "imgui.h"
#include "../utils/Log.h"
#include "../utils/FileUtils.h"

ProjectWindow::ProjectWindow(bool active) : UIWindow("Project", active)
{

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

    std::string path = "Assets";

    for (std::string path : GetListDirectoryContents(path))
    {
        ImGui::Text("%s", path.c_str());
    }

    ImGui::End();
}