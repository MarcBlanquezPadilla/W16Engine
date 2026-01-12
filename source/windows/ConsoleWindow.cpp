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
        bool showingInfo = !info;
        if (showingInfo) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
        if (ImGui::MenuItem("Info"))
        {
            info = !info;
        }
        if (showingInfo) ImGui::PopStyleColor();

        bool showingWarning = !warning;
        if (showingWarning) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
        if (ImGui::MenuItem("Warning"))
        {
            warning = !warning;
        }
        if (showingWarning) ImGui::PopStyleColor();

        bool showingError = !error;
        if (showingError) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
        if (ImGui::MenuItem("Error"))
        {
            error = !error;
        }
        if (showingError) ImGui::PopStyleColor();
        if (ImGui::MenuItem("Clear"))
        {
            LogBuffer::GetInstance().Clear();
        }

        ImGui::EndMenuBar();
    }

    if (ImGui::BeginTable("ConsoleTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
    {
        ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed, 40.0f);

        for (const auto& log : LogBuffer::GetInstance().GetLogs())
        {
            if (log.type == LogType::LOG_INFO && !info) continue;
            if (log.type == LogType::LOG_WARNING && !warning) continue;
            if (log.type == LogType::LOG_ERROR && !error) continue;

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);

            ImVec4 textColor = { 1, 1, 1, 1 };
            if (log.type == LogType::LOG_WARNING) textColor = { 1.0f, 1.0f, 0.0f, 1.0f };
            if (log.type == LogType::LOG_ERROR)   textColor = { 1.0f, 0.3f, 0.3f, 1.0f };

            ImGui::PushStyleColor(ImGuiCol_Text, textColor);

            ImGui::TextWrapped("%s", log.message.c_str());

            ImGui::PopStyleColor();

            if (log.count > 1)
            {
                ImGui::TableSetColumnIndex(1);

                ImGui::TextDisabled("%d", log.count);
            }
        }

        ImGui::EndTable();
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::End();
}