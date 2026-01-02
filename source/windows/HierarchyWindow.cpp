#include "HierarchyWindow.h"
#include "../Engine.h"
#include "../ModuleScene.h"
#include "../ModuleLoader.h"
#include "../ModuleEditor.h"
#include "../ModuleInput.h"
#include "../Global.h"
#include "../GameObject.h"
#include "imgui.h"
#include "../utils/Log.h"

HierarchyWindow::HierarchyWindow(bool active) : UIWindow("Hierarchy", active)
{
    isFocused = false;
}

HierarchyWindow::~HierarchyWindow()
{
    
}

void HierarchyWindow::Draw()
{
    isFocused = false;

    if (!is_active) return;

    if (!ImGui::Begin(name, &is_active))
    {
        ImGui::End();
        return;
    }

    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        ImGui::SetWindowFocus();
    }

    isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

    ModuleScene* scene = Engine::GetInstance().moduleScene;
    ModuleLoader* loader = Engine::GetInstance().moduleLoader;

    for (GameObject* go : scene->GetRootGameObjects())
    {
        DrawGameObjectNode(go);
    }

    if (isFocused && ImGui::BeginPopupContextWindow("HierarchyContextMenu"))
    {
        if (ImGui::BeginMenu("Create"))
        {
            if (ImGui::MenuItem("Empty")) { loader->LoadEmpty(); }
            if (ImGui::MenuItem("Cube")) { loader->LoadBasic(CUBE); }
            if (ImGui::MenuItem("Sphere")) { loader->LoadBasic(SPHERE); }
            if (ImGui::MenuItem("Pyramid")) { loader->LoadBasic(PYRAMID); }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }

    ImVec2 contentSize = ImGui::GetContentRegionAvail();
    ImGui::Dummy(contentSize);

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(GAMEOBJECTS_DRAG))
        {
            uint32_t* uidsIdx = (uint32_t*)payload->Data;
            int objectCount = payload->DataSize / sizeof(uint32_t);

            for (int i = 0; i < objectCount; i++)
            {
                uint32_t draggedUID = uidsIdx[i];
                GameObject* draggedGO = Engine::GetInstance().moduleScene->GetObjectByUUID(draggedUID);

                if (draggedGO != nullptr)
                {
                    draggedGO->SetParent(nullptr);
                }
            }
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::End();
}

void HierarchyWindow::DrawGameObjectNode(GameObject* go)
{
    if (go == nullptr) return;

    const std::vector<GameObject*>& selectedObjects = Engine::GetInstance().moduleEditor->GetSelectedGameObjects();

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

    bool ctrl = Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT;
    bool shift = Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT;
    bool erase = !(ctrl || shift);

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
    {
        if (!isSelected)
        {
            Engine::GetInstance().moduleEditor->SetSelected(go, erase);
        }
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0))
    {
        if (!ImGui::IsMouseDragging(0) && !ctrl && isSelected)
        {
            Engine::GetInstance().moduleEditor->SetSelected(go, erase);
        }
    }

    if (ImGui::BeginDragDropSource())
    {
        if (!isSelected) {
            Engine::GetInstance().moduleEditor->SetSelected(go, true);
        }

        const std::vector<GameObject*>& currentSelection = Engine::GetInstance().moduleEditor->GetSelectedGameObjects();

        std::vector<uint32_t> uidsToSend;
        uidsToSend.reserve(currentSelection.size());

        for (GameObject* s : currentSelection)
        {
            uidsToSend.push_back(s->UUID);
        }

        ImGui::SetDragDropPayload(GAMEOBJECTS_DRAG, uidsToSend.data(), uidsToSend.size() * sizeof(uint32_t));

        ImGui::Text("Moviendo %d objetos", (int)uidsToSend.size());

        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(GAMEOBJECTS_DRAG))
        {
            uint32_t* uidsIdx = (uint32_t*)payload->Data;
            int objectCount = payload->DataSize / sizeof(uint32_t);

            for (int i = 0; i < objectCount; i++)
            {
                uint32_t draggedUID = uidsIdx[i];

                GameObject* draggedGO = Engine::GetInstance().moduleScene->GetObjectByUUID(draggedUID);

                if (draggedGO != nullptr)
                {
                    if (draggedGO != go && !go->IsDescendant(draggedGO))
                    {
                        draggedGO->SetParent(go);
                    }
                }
            }
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