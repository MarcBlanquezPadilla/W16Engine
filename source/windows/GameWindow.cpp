#include "GameWindow.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "../Engine.h"
#include "../CameraLens.h"
#include "../ModuleRender.h"
#include "../ModuleScene.h"
#include "../GameObject.h"

GameWindow::GameWindow(bool active) : UIWindow("Game", active)
{
    
}

GameWindow::~GameWindow()
{
    
}

void GameWindow::Draw()
{
    if (!is_active) return;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    if (!ImGui::Begin(name, &is_active))
    {
        ImGui::End();
        ImGui::PopStyleVar();
        return;
    }

	ImGui::PopStyleVar();

    CameraLens* cam = Engine::GetInstance().moduleRender->GetMainCamera();
    int camNum = Engine::GetInstance().moduleRender->GetMainCamerasNum();
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();

    const char* text = "";

    if (cam)
    {
        if (camNum > 1) 
            text = "There's more than one main camera";

        //TEXTURE 
        if (viewportSize.x != cam->textureWidth || viewportSize.y != cam->textureHeight)
        {
            cam->SetRenderTarget((int)viewportSize.x, (int)viewportSize.y);
            cam->SetPerspective(cam->GetFov(), viewportSize.x / viewportSize.y, cam->GetNearPlane(), cam->GetFarPlane());
        }

        unsigned int textureID = cam->textureID;

        ImVec2 winPos = ImGui::GetCursorScreenPos();

        ImGui::Image((ImTextureID)(intptr_t)textureID, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
    }
    else
    {
        text = "There's no main camera in scene";
    }

    if (text != "")
    {
        ImVec2 textSize = ImGui::CalcTextSize(text);

        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        float textX = windowPos.x + (windowSize.x - textSize.x) * 0.5f;
        float textY = windowPos.y + (windowSize.y - textSize.y) * 0.5f;

        ImGui::SetCursorScreenPos(ImVec2(textX, textY));

        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), text);
    }
    

    ImGui::End();
}