#include "InspectorWindow.h"
#include "../Engine.h"
#include "../Scene.h"
#include "../GameObject.h"
#include "../components/Component.h"
#include "../components/Transform.h"
#include "../components/Mesh.h"
#include "../components/Texture.h"
#include "../Log.h"
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

    GameObject* gameObject = Engine::GetInstance().scene->GetSelectedGameObject();
    Editor* editor = Engine::GetInstance().editor;

    if (gameObject != nullptr)
    {
        char name_buffer[64];
        sprintf_s(name_buffer, "%s", gameObject->name.c_str());
        ImGui::Checkbox(name_buffer, &gameObject->enabled);
        ImGui::Separator();
        for each(auto const& pair in gameObject->components)
        {
            switch (pair.first)
            {
            case ComponentType::Transform:
                if (ImGui::CollapsingHeader("Transform"))
                {
                    Transform* transform = (Transform*)pair.second;
                    if (transform)
                    {
                        //GIZMO MODE SELECTION
                        const char* gizmoNames[] = { "Translate", "Rotate", "Scale" };
                        static int gizmoIndex = 0;

                        if (editor->currentGizmoOperation == ImGuizmo::TRANSLATE) gizmoIndex = 0;
                        if (editor->currentGizmoOperation == ImGuizmo::ROTATE) gizmoIndex = 1;
                        if (editor->currentGizmoOperation == ImGuizmo::SCALE) gizmoIndex = 2;

                        if (ImGui::Combo("Gizmo Mode", &gizmoIndex, gizmoNames, IM_ARRAYSIZE(gizmoNames)))
                        {
                            switch (gizmoIndex)
                            {
                            case 0: editor->currentGizmoOperation = ImGuizmo::TRANSLATE; break;
                            case 1: editor->currentGizmoOperation = ImGuizmo::ROTATE;    break;
                            case 2: editor->currentGizmoOperation = ImGuizmo::SCALE;     break;
                            }
                        }
                        ImGui::Separator();

                        //ATRIBUTES
                        ImGui::Text("Position");
                        glm::vec3 current_position = transform->GetPosition();
                        if (ImGui::InputFloat3("##Pos", &current_position.x))
                        {
                            transform->SetPosition(current_position);
                        }


                        ImGui::Text("Rotation");
                        glm::vec3 current_euler_degrees = transform->GetEulerRotation();
                        if (ImGui::InputFloat3("##Rot", &current_euler_degrees.x))
                        {
                            transform->SetEulerRotation(current_euler_degrees);
                        }

                        ImGui::Text("Scale");
                        glm::vec3 current_scale = transform->GetScale();
                        if(ImGui::InputFloat3("##Scale", &current_scale.x))
                        {
                            transform->SetScale(current_scale);
                        }
                    }
                }
                break;
            case ComponentType::Mesh:
                if (ImGui::CollapsingHeader("Mesh"))
                {
                    Mesh* mesh = (Mesh*)pair.second;

                    if (mesh)
                    {
                        ImGui::Text("Vertices:");
                        ImGui::SameLine(); 
                        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.0f, 1.0f), "%d", mesh->meshData.numVertices);

                        ImGui::Text("Indices:");
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.0f, 1.0f), "%d", mesh->meshData.numIndices);

                        ImGui::Text("VAO (ID):");
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.9f, 1.0f), "%u", mesh->meshData.VAO);

                        ImGui::Text("Has UVs:");
                        ImGui::SameLine();
                        ImGui::TextUnformatted(mesh->hasUVs ? "Yes" : "No");

                        ImGui::Separator();
                        ImGui::Checkbox("Draw Normals", &mesh->drawNormals);
                    }
                }
                break;
            case ComponentType::Texture:
                if (ImGui::CollapsingHeader("Texture"))
                {
                    Texture* texture = (Texture*)pair.second;
                    if (texture)
                    {
                        ImGui::Text("Path:");
                        ImGui::TextWrapped(texture->path.c_str());

                        ImGui::Text("Size:");
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.9f, 1.0f), "%dx%d", texture->width, texture->height);
                        ImGui::Text("Texture ID (GPU):");
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.9f, 1.0f), "%u", texture->textureID);
                        ImGui::Separator();
                        ImGui::Checkbox("Use Checker Texture", &texture->use_checker);
                        ImGui::Checkbox("Transparent", &texture->transparent);
                    }
                }
                break;
            }
        }
    }
    ImGui::End();
}