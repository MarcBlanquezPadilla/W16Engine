#include "AboutWindow.h"
#include "imgui.h"
#include "../Global.h"

AboutWindow::AboutWindow(bool active) : UIWindow("About", active)
{

}

AboutWindow::~AboutWindow()
{
    
}

void AboutWindow::Draw()
{
    if (!is_active) return;

    if (!ImGui::Begin(name, &is_active))
    {
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Motor"))
    {
        ImGui::Text("Name: %s", NAME);
        ImGui::Text("Version: %s", VERSION);
    }

    if (ImGui::CollapsingHeader("Developers"))
    {
        ImGui::Text("%s", DEVELOPER_1);
        ImGui::Text("%s", DEVELOPER_2);
        ImGui::Text("%s", DEVELOPER_3);
    }

    if (ImGui::CollapsingHeader("Libraries"))
    {
        ImGui::Text("%s", LIBRARY_1);
        ImGui::Text("%s", LIBRARY_2);
        ImGui::Text("%s", LIBRARY_3);
        ImGui::Text("%s", LIBRARY_4);
        ImGui::Text("%s", LIBRARY_5);
        ImGui::Text("%s", LIBRARY_6);
    }
    if (ImGui::CollapsingHeader("License"))
    {
        ImGui::Text("Copyright (c) 2025 TorratsGames\n"
            "\n"
            "Permission is hereby granted, free of charge, to any person obtaining a copy\n"
            "of this software and associated documentation files (the \"Software\"), to deal\n"
            "in the Software without restriction, including without limitation the rights\n"
            "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
            "copies of the Software, and to permit persons to whom the Software is\n"
            "furnished to do so, subject to the following conditions:\n"
            "\n"
            "The above copyright notice and this permission notice shall be included in all\n"
            "copies or substantial portions of the Software.\n"
            "\n"
            "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
            "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
            "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
            "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
            "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
            "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
            "SOFTWARE.\n");
    }


    ImGui::End();
}