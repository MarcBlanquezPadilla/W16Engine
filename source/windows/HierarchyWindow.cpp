#include "HierarchyWindow.h"
#include "../Engine.h"
#include "../Scene.h"
#include "../Loader.h"
#include "../Editor.h"
#include "../Input.h"
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
        dragging = false;
        ImGui::End();
        return;
    }

    Scene* scene = Engine::GetInstance().scene;
    Loader* loader = Engine::GetInstance().loader;

    objectToDrop = nullptr;

    for (GameObject* go : scene->GetGameObjects())
    {
        DrawGameObjectNode(go);
    }

    if (ImGui::BeginPopupContextWindow("HierarchyContextMenu"))
    {
        if (ImGui::BeginMenu("Create"))
        {
            if (ImGui::MenuItem("Empty")) { loader->CreateBasic(EMPTY); }
            if (ImGui::MenuItem("Cube")) { loader->CreateBasic(CUBE); }
            if (ImGui::MenuItem("Sphere")) { loader->CreateBasic(SPHERE); }
            if (ImGui::MenuItem("Pyramid")) { loader->CreateBasic(PYRAMID); }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }

    ImVec2 contentSize = ImGui::GetContentRegionAvail();
    ImVec2 p_min = ImGui::GetCursorScreenPos();
    ImVec2 p_max = ImVec2(p_min.x + contentSize.x, p_min.y + contentSize.y);
    ImGui::Dummy(contentSize);

    const std::vector<GameObject*>& selectedObjects = Engine::GetInstance().editor->GetSelectedGameObjects();

    if (dragging && ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly))
    {
        ImGui::GetWindowDrawList()->AddRect(
            p_min,
            p_max,
            IM_COL32(0, 255, 255, 255),
            0.0f,
            0,
            1.0f
        );
    }

    if (dragging && ImGui::IsMouseReleased(0))
    {
        if (objectToDrop != nullptr)
        {
            for (GameObject* movingObj : selectedObjects)
            {
                if (movingObj != objectToDrop && !objectToDrop->IsDescendant(movingObj))
                {
                    movingObj->SetParent(objectToDrop);
                }
            }
        }
        else if (ImGui::IsWindowHovered())
        {
            for (GameObject* movingObj : selectedObjects)
            {
                movingObj->SetParent(nullptr);
            }
        }

        dragging = false;
        objectToDrop = nullptr;
    }

    if (dragging && !selectedObjects.empty())
    {
        ImGui::SetTooltip("Moving %d objects", selectedObjects.size());
    }
    else if (dragging && selectedObjects.empty())
    {
        dragging = false;
        objectToDrop = nullptr;
    }

    ImGui::End();
}

void HierarchyWindow::DrawGameObjectNode(GameObject* go)
{
    if (go == nullptr) return;

    const std::vector<GameObject*>& selectedObjects = Engine::GetInstance().editor->GetSelectedGameObjects();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

    if (go->childs.empty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    bool isSelected = std::find(selectedObjects.begin(), selectedObjects.end(), go) != selectedObjects.end();

    if (isSelected)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    bool node_open = ImGui::TreeNodeEx((void*)go, flags, go->name.c_str());

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
    {
        objectToSelect = go;
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0) && !dragging)
    {
        bool ctrlPressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT;
        bool shiftPressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT;
        bool eraseSelecteds = !(ctrlPressed || shiftPressed);

        if (objectToSelect == go) Engine::GetInstance().editor->SetSelected(go, eraseSelecteds);
        objectToSelect = nullptr;
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseDragging(0) && !dragging)
    {
        bool ctrlPressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT;
        bool shiftPressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT;
        bool eraseSelecteds = !(ctrlPressed || shiftPressed);
        if (!isSelected) Engine::GetInstance().editor->SetSelected(go, eraseSelecteds);
        dragging = true;
    }

    if (dragging && ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly))
    {
        objectToDrop = go;
        ImGui::GetWindowDrawList()->AddRect(
            ImGui::GetItemRectMin(),
            ImGui::GetItemRectMax(),
            IM_COL32(0, 255, 255, 255),
            0.0f,
            0,
            1.0f
        );
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