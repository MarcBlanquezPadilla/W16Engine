#include "ConsoleWindow.h"
#include "imgui.h"
#include "../Log.h"

ConsoleWindow::ConsoleWindow() : UIWindow("Console")
{

}

ConsoleWindow::~ConsoleWindow()
{
    
}

void ConsoleWindow::Draw()
{
    if (!is_active) return;

    if (!ImGui::Begin(name, &is_active))
    {
        ImGui::End();
        return;
    }

    for each(std::string message in LogBuffer::GetInstance().GetMessages())
    {
        ImGui::Text("%s", message.c_str());
    }

    ImGui::End();
}