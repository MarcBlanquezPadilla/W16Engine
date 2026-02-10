#include "InspectorWindow.h"
#include "../Engine.h"
#include "../ModuleScene.h"
#include "../ModuleEvents.h"
#include "../GameObject.h"
#include "../components/Component.h"
#include "../components/Transform.h"
#include "../components/MeshRenderer.h"
#include "../utils/Log.h"
#include "../ModuleEditor.h"

#include "imgui.h"
#include "ImGuizmo.h"


InspectorWindow::InspectorWindow(bool active) : UIWindow("Inspector", active)
{
    Engine::GetInstance().moduleEvents->Subscribe(Event::Type::GameObjectDestroyed, this);
}

InspectorWindow::~InspectorWindow()
{

}

void InspectorWindow::CleanUp()
{
    Engine::GetInstance().moduleEvents->UnsubscribeAll(this);
}

void InspectorWindow::Draw()
{
    if (!is_active) return;

    if (!ImGui::Begin(name, &is_active))
    {
        ImGui::End();
        return;
    }

    std::string lockText = "Lock";


    int size = ImGui::CalcTextSize(lockText.c_str()).x + ImGui::GetFrameHeight();
    int startWidth = ImGui::GetContentRegionAvail().x - size;

    ImGui::SetCursorPosX(startWidth);
    ImGui::Text(lockText.c_str());
    ImGui::SameLine();
    
    if (ImGui::Checkbox("##LockInspector", &inspectorLocked))
    {
        if (inspectorLocked)
        {
            lockedObjects = Engine::GetInstance().moduleEditor->GetSelectedGameObjects();
        }
    }

    ImGui::Separator();
    ImGui::Spacing();

    const std::vector<GameObject*>& selectedObjects = inspectorLocked ? lockedObjects : Engine::GetInstance().moduleEditor->GetSelectedGameObjects();

    if (selectedObjects.empty())
    {
        ImGui::Text("No object selected");
    }
    else if (selectedObjects.size() == 1)
    {
        selectedObjects[0]->OnEditor();
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
                go->OnEditor();
            }

            ImGui::PopID();
        }
    }

    ImGui::End();
}

void InspectorWindow::OnEvent(const Event& event)
{
    switch (event.type)
    {
    case Event::Type::GameObjectDestroyed:
    {
        GameObject* destroyedGO = event.data.gameObject.gameObject;

        auto it = std::remove(lockedObjects.begin(), lockedObjects.end(), destroyedGO);

        if (it != lockedObjects.end())
        {
            lockedObjects.erase(it, lockedObjects.end());
        }
        break;
    }
    default:
        break;
    }
}