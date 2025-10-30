#include "ConfigWindow.h"
#include "imgui.h"
#include "../Engine.h"
#include "../Render.h"
#include "../Window.h"

ConfigWindow::ConfigWindow(bool active) : UIWindow("Configuration", active)
{
    fps_log.resize(100, 0.0f);
    memory_log.resize(100, 0.0f);
}

ConfigWindow::~ConfigWindow()
{
    fps_log.clear();
    memory_log.clear();
}

void ConfigWindow::Draw()
{
    if (!is_active) return;

    if (!ImGui::Begin(name, &is_active))
    {
        ImGui::End();
        return;
    }

    //FPS
    if (ImGui::CollapsingHeader("Framerate"))
    {
        float current_fps = ImGui::GetIO().Framerate;
        float current_ms = ImGui::GetIO().DeltaTime;

        fps_log.erase(fps_log.begin());
        fps_log.push_back(current_fps);

        char title[25];
        sprintf_s(title, "FPS: %.1f   MS: %.1f", current_fps, current_ms * 1000);

        ImGui::PlotLines(
            "##fps",
            &fps_log[0],
            (int)fps_log.size(),
            0,
            title,
            0.0f,
            150.0f,
            ImVec2(-1.0f, 50)
        );
    }

    if (ImGui::CollapsingHeader("Memory"))
    {
        float ram_mb = 0;
        HANDLE process = GetCurrentProcess();

        if (GetProcessMemoryInfo(process, &mem_counters, sizeof(mem_counters)))
        {
            ram_mb = mem_counters.WorkingSetSize / (1024 * 1024);
        }
        CloseHandle(process);

        memory_log.erase(memory_log.begin());
        memory_log.push_back(ram_mb);

        char title[25];
        sprintf_s(title, "MEMORY USAGE: %.1f MB", ram_mb);

        ImGui::PlotLines(
            "##ram",
            &memory_log[0],
            (int)memory_log.size(),
            0,
            title,
            0.0f,
            150.0f,
            ImVec2(-1.0f, 50)
        );
    }

    if (ImGui::CollapsingHeader("Hardware & Versions"))
    {
        ImGui::Text("SDL Version: %s", Engine::GetInstance().window->GetSDLVersion().c_str());
        ImGui::Text("OpenGL Version: %s", Engine::GetInstance().render->GetGLVersion().c_str());
        ImGui::Text("GLSL Version: %s", Engine::GetInstance().render->GetGLSLVersion().c_str());
        ImGui::Text("CPU: %s", Engine::GetInstance().window->GetCPU().c_str());
        ImGui::Text("RAM: %s", Engine::GetInstance().window->GetRAM().c_str());
        ImGui::Text("GPU: %s", Engine::GetInstance().render->GetGPU().c_str());
    }

    ImGui::End();
}