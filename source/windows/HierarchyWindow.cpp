#include "HierarchyWindow.h"
#include "../Engine.h"
#include "../Scene.h"
#include "../Global.h"
#include "../GameObject.h"
#include "imgui.h"
#include "../Log.h"

HierarchyWindow::HierarchyWindow(bool active) : UIWindow("Hierarchy", active)
{

}

HierarchyWindow::~HierarchyWindow()
{
    
}

void HierarchyWindow::Draw()
{
    if (!is_active) return;

    if (!ImGui::Begin(name, &is_active))
    {
        ImGui::End();
        return;
    }

    for each(GameObject * gameObject in Engine::GetInstance().scene->GetGameObjects())
    {
        bool isSelected = (gameObject == Engine::GetInstance().scene->GetSelectedGameObject());
        if (ImGui::Selectable(gameObject->name.c_str(), isSelected))
        {
            Engine::GetInstance().scene->SetSelectedGameObject(gameObject);
        }
    }

    if (ImGui::BeginPopupContextWindow("HierarchyContextMenu"))
    {
        if (ImGui::BeginMenu("Create"))
        {
            if (ImGui::MenuItem("Cube"))
            {
                Engine::GetInstance().scene->CreateBasic(CUBE);
            }
            if (ImGui::MenuItem("Sphere"))
            {
                Engine::GetInstance().scene->CreateBasic(SPHERE);
            }
            if (ImGui::MenuItem("Pyramid"))
            {
                Engine::GetInstance().scene->CreateBasic(PYRAMID);
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}