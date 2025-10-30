#include "HierarchyWindow.h"
#include "../Engine.h"
#include "../Scene.h"
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

    ImGui::End();
}