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
            LogBuffer::GetInstance().Clear();
        }

        ImGui::EndMenuBar();
    }

    if (ImGui::Button("Info")) info = !info;
    if (ImGui::Button("Waring")) warning = !warning;
    if (ImGui::Button("Error")) error = !error;

    for (LogInfo Log : LogBuffer::GetInstance().GetLogs())
    {
        if (Log.type == LOG_INFO && !info) continue;
        if (Log.type == LOG_WARNING && !warning) continue;
        if (Log.type == LOG_ERROR && !error) continue;

        ImGui::Text("%d | %s", Log.count, Log.message.c_str());
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::End();
}