#include "SceneWindow.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "../Engine.h"
#include "../Editor.h"
#include "../CameraLens.h"
#include "../EditorCamera.h"
#include "../Render.h"
#include "../Input.h"
#include "../utils/Log.h"
#include "../components/Transform.h"
#include "../GameObject.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

SceneWindow::SceneWindow(bool active) : UIWindow("Scene", active)
{
    
}

SceneWindow::~SceneWindow()
{
    
}

void SceneWindow::Draw()
{
    if (!is_active) return;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    if (!ImGui::Begin(name, &is_active, ImGuiWindowFlags_MenuBar))
    {
        ImGui::End();
        ImGui::PopStyleVar();
        return;
    }

	ImGui::PopStyleVar();

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Debug"))
		{
            ImGui::MenuItem("Ray", NULL, &Engine::GetInstance().editor->debugRay);
            ImGui::MenuItem("Mesh", NULL, &Engine::GetInstance().editor->debugMesh);
            ImGui::MenuItem("AABB", NULL, &Engine::GetInstance().editor->debugAABB);

            ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
    
    CameraLens* cam = Engine::GetInstance().editor->GetEditorCameraLens();
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();

    //TEXTURE 
    if (viewportSize.x != cam->textureWidth || viewportSize.y != cam->textureHeight)
    {
        cam->SetRenderTarget((int)viewportSize.x, (int)viewportSize.y);
        cam->SetPerspective(cam->fov, viewportSize.x / viewportSize.y, cam->zNear, cam->zFar);
    }

    unsigned int textureID = cam->textureID;

    ImVec2 winPos = ImGui::GetCursorScreenPos();

    ImGui::Image((ImTextureID)(intptr_t)textureID, viewportSize, ImVec2(0, 1), ImVec2(1, 0));

    //ENABLE CAMERA CONTROLS
    Engine::GetInstance().GetInstance().editor->GetEditorCamera()->LockCamera(!ImGui::IsWindowHovered());

    //PICKING
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !ImGuizmo::IsOver())
    {
        ImVec2 mousePos = ImGui::GetMousePos();
        int mouseX = (int)(mousePos.x - winPos.x);
        int mouseY = (int)(mousePos.y - winPos.y);

        Engine::GetInstance().editor->TestMouseRay(mouseX, mouseY, (int)viewportSize.x, (int)viewportSize.y);
    }

	GameObject* selectedGameObject = Engine::GetInstance().editor->GetSelectedGameObject();

	//GUIZMO
	if (selectedGameObject != nullptr)
	{
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_DOWN) Engine::GetInstance().editor->currentGizmoOperation = ImGuizmo::TRANSLATE;
		else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_E) == KEY_DOWN) Engine::GetInstance().editor->currentGizmoOperation = ImGuizmo::SCALE;
		else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_R) == KEY_DOWN) Engine::GetInstance().editor->currentGizmoOperation = ImGuizmo::ROTATE;

		Transform* transform = (Transform*)selectedGameObject->GetComponent(ComponentType::Transform);
		if (transform)
		{
            ImGuizmo::SetOrthographic(false);

            ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

            ImGuizmo::SetRect(winPos.x, winPos.y, viewportSize.x, viewportSize.y);

            const glm::mat4& viewMatrix = cam->GetViewMatrix();
            const glm::mat4& projectionMatrix = cam->GetProjectionMatrix();
            glm::mat4 modelMatrix = transform->GetLocalMatrix();

            ImGuizmo::Manipulate(
                glm::value_ptr(viewMatrix),
                glm::value_ptr(projectionMatrix),
                Engine::GetInstance().editor->currentGizmoOperation,
                ImGuizmo::LOCAL,
                glm::value_ptr(modelMatrix)
            );

			bool isGuizmoUsing = ImGuizmo::IsUsing();
			Engine::GetInstance().editor->GetEditorCamera()->LockCamera(isGuizmoUsing);
			if (isGuizmoUsing)
			{
				glm::vec3 newPos, newEulerRot, newScale;

				ImGuizmo::DecomposeMatrixToComponents(
					glm::value_ptr(modelMatrix),
					glm::value_ptr(newPos),
					glm::value_ptr(newEulerRot),
					glm::value_ptr(newScale)
				);

				transform->SetPosition(newPos);
				transform->SetEulerRotation(newEulerRot);
				transform->SetScale(newScale);
			}
		}
	}

    ImGui::End();
}