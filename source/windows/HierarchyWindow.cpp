#include "HierarchyWindow.h"
#include "../Engine.h"
#include "../Scene.h"
#include "../Loader.h"
#include "../Editor.h"
#include "../Global.h"
#include "../GameObject.h"
#include "imgui.h"
#include "../utils/Log.h"

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

    Scene* scene = Engine::GetInstance().scene;
    Loader* loader = Engine::GetInstance().loader;

    for (GameObject* go : scene->GetGameObjects())
    {
        DrawGameObjectNode(go);
    }

    if (ImGui::BeginPopupContextWindow("HierarchyContextMenu"))
    {
        if (ImGui::BeginMenu("Create"))
        {
            if (ImGui::MenuItem("Cube")) { loader->CreateBasic(CUBE); }
            if (ImGui::MenuItem("Sphere")) { loader->CreateBasic(SPHERE); }
            if (ImGui::MenuItem("Pyramid")) { loader->CreateBasic(PYRAMID); }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }

    ImVec2 contentSize = ImGui::GetContentRegionAvail();

    ImGui::Dummy(contentSize);

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_NODE"))
        {
            GameObject* droppedGO = *(GameObject**)payload->Data;

            draggedGameObject = droppedGO;
            targetGameObject = nullptr;
            toRoot = true;
        }
        ImGui::EndDragDropTarget();
    }

    if (draggedGameObject != nullptr)
    {
        if (toRoot)
        {
            draggedGameObject->SetParent(nullptr);
        }
        else if (targetGameObject != nullptr)
        {
            draggedGameObject->SetParent(targetGameObject);
        }

        draggedGameObject = nullptr;
        targetGameObject = nullptr;
        toRoot = false;
    }

    ImGui::End();
}

void HierarchyWindow::DrawGameObjectNode(GameObject* go)
{
    if (go == nullptr) return;

    GameObject* selected_go = Engine::GetInstance().editor->GetSelectedGameObject();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

    if (go->childs.empty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (go == selected_go)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    bool node_open = ImGui::TreeNodeEx((void*)go, flags, go->name.c_str());

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        Engine::GetInstance().editor->SetSelected(go);
    }

    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("HIERARCHY_NODE", &go, sizeof(GameObject*));

        ImGui::Text("Moving %s", go->name.c_str());

        draggedGameObject = go;

        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_NODE"))
        {
            GameObject* droppedGO = *(GameObject**)payload->Data;

            draggedGameObject = droppedGO;
            targetGameObject = go;
            toRoot = false;
        }
        ImGui::EndDragDropTarget();
    }

    if (node_open)
    {
        for (GameObject* child : go->childs)
        {
            DrawGameObjectNode(child);
        }
        ImGui::TreePop();
    }
}