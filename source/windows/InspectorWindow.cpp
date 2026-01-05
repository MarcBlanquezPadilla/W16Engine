#include "InspectorWindow.h"
#include "../Engine.h"
#include "../ModuleScene.h"
#include "../GameObject.h"
#include "../components/Component.h"
#include "../components/Transform.h"
#include "../components/Mesh.h"
#include "../components/Texture.h"
#include "../utils/Log.h"
#include "../ModuleEditor.h"

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

    const std::vector<GameObject*>& selectedObjects = Engine::GetInstance().moduleEditor->GetSelectedGameObjects();

    if (selectedObjects.empty())
    {
        ImGui::Text("No object selected");
    }
    else if (selectedObjects.size() == 1)
    {
        DrawGameObjectInfo(selectedObjects[0]);
    }
    else
    {
        ImGui::Text("%d Objects Selected", selectedObjects.size());
        ImGui::Separator();

        for (GameObject* go : selectedObjects)
        {
            ImGui::PushID(go);

            if (ImGui::CollapsingHeader(go->name.c_str()))
            {
                DrawGameObjectInfo(go);
            }

            

            ImGui::PopID();
        }
    }

    ImGui::End();
}

void InspectorWindow::DrawGameObjectInfo(GameObject* gameObject)
{
    if (gameObject == nullptr) return;

    char name_buffer[256];
    sprintf_s(name_buffer, "%s", gameObject->name.c_str());

    if (ImGui::InputText("Name", name_buffer, sizeof(name_buffer)))
    {
        gameObject->name = name_buffer;
    }

    bool isEnabled = gameObject->GetEnabled();
    if (ImGui::Checkbox("Enabled", &isEnabled))
    {
        gameObject->SetEnabled(isEnabled);
    }

    ImGui::SameLine();

    bool isStatic = gameObject->GetStatic();
    if (ImGui::Checkbox("Static", &isStatic))
    {
        gameObject->SetStatic(isStatic);
    }

    for (auto const& pair : gameObject->components)
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
            if (ImGui::MenuItem("Camera")) { gameObject->AddComponent(ComponentType::Camera); ImGui::CloseCurrentPopup(); }
        }
        if (gameObject->GetComponent(ComponentType::Mesh) == nullptr)
        {
            if (ImGui::MenuItem("Mesh")) { gameObject->AddComponent(ComponentType::Mesh); ImGui::CloseCurrentPopup(); }
        }
        if (gameObject->GetComponent(ComponentType::Texture) == nullptr)
        {
            if (ImGui::MenuItem("Texture")) { gameObject->AddComponent(ComponentType::Texture); ImGui::CloseCurrentPopup(); }
        }
        if (gameObject->GetComponent(ComponentType::Animation) == nullptr)
        {
            if (ImGui::MenuItem("Animation")) { gameObject->AddComponent(ComponentType::Animation); ImGui::CloseCurrentPopup(); }
        }

        ImGui::EndPopup();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();
}