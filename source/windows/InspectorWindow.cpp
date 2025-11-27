#include "InspectorWindow.h"
#include "../Engine.h"
#include "../Scene.h"
#include "../GameObject.h"
#include "../components/Component.h"
#include "../components/Transform.h"
#include "../components/Mesh.h"
#include "../components/Texture.h"
#include "../utils/Log.h"
#include "../Editor.h"

#include "imgui.h"
#include "ImGuizmo.h"


InspectorWindow::InspectorWindow(bool active) : UIWindow("Inspector", active)
{

}

InspectorWindow::~InspectorWindow()
{
    
}

void InspectorWindow::Draw()
{
    if (!is_active) return;

    if (!ImGui::Begin(name, &is_active))
    {
        ImGui::End();
        return;
    }

    GameObject* gameObject = Engine::GetInstance().editor->GetSelectedGameObject();
    Editor* editor = Engine::GetInstance().editor;

    if (gameObject != nullptr)
    {
        ImGui::Text(gameObject->name.c_str());
        bool isEnabled = gameObject->GetEnabled();
        if (ImGui::Checkbox("Enabled", &isEnabled))
        {
            gameObject->SetEnabled(isEnabled);
        }
        
        bool isStatic = gameObject->GetStatic();
        if (ImGui::Checkbox("Static", &isStatic))
        {
            gameObject->SetStatic(isStatic);
        }

        ImGui::Separator();

        for each(auto const& pair in gameObject->components)
        {
            if (pair.second) pair.second->OnEditor();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        float buttonWidth = ImGui::GetContentRegionAvail().x * 0.6f;
        float centerPos = (ImGui::GetContentRegionAvail().x - buttonWidth) * 0.5f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + centerPos);

        if (ImGui::Button("Add Component", ImVec2(buttonWidth, 0)))
        {
            ImGui::OpenPopup("AddComponentPopup");
        }

        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            if (gameObject->GetComponent(ComponentType::Camera) == nullptr)
            {
                if (ImGui::MenuItem("Camera"))
                {
                    gameObject->AddComponent(ComponentType::Camera);
                    ImGui::CloseCurrentPopup();
                }
            }
            else
            {
                ImGui::BeginDisabled();
                ImGui::MenuItem("Camera (Added)");
                ImGui::EndDisabled();
            }
            if (gameObject->GetComponent(ComponentType::Mesh) == nullptr)
            {
                if (ImGui::MenuItem("Mesh"))
                {
                    gameObject->AddComponent(ComponentType::Mesh);
                    ImGui::CloseCurrentPopup();
                }
            }
            if (gameObject->GetComponent(ComponentType::Texture) == nullptr)
            {
                if (ImGui::MenuItem("Texture"))
                {
                    gameObject->AddComponent(ComponentType::Texture);
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndPopup();
        }
    }
    ImGui::End();
}