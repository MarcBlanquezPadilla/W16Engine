#include "InspectorWindow.h"
#include "../Engine.h"
#include "../ModuleScene.h"
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