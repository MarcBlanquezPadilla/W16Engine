#include "ConsoleWindow.h"
#include "imgui.h"
#include "../utils/Log.h"

ConsoleWindow::ConsoleWindow(bool active) : UIWindow("Console", active)
{

}

ConsoleWindow::~ConsoleWindow()
{
    
}

void ConsoleWindow::Draw()
{
    if (!is_active) return;

    if (!ImGui::Begin(name, &is_active, ImGuiWindowFlags_MenuBar))
    {
        ImGui::End();
        return;
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::MenuItem("Clear"))
        {
            LogBuffer::GetInstance().EraseMessages();
        }

        ImGui::EndMenuBar();
    }

    for (std::string message : LogBuffer::GetInstance().GetMessages())
    {
        ImGui::Text("%s", message.c_str());
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::End();
}