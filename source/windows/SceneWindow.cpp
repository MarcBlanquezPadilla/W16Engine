#include "SceneWindow.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "../Engine.h"
#include "../Editor.h"
#include "../CameraLens.h"
#include "../EditorCamera.h"
#include "../Render.h"
#include "../Window.h"
#include "../Input.h"
#include "../utils/Log.h"
#include "../components/Transform.h"
#include "../GameObject.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/matrix_decompose.hpp"

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
			ImGui::MenuItem("Grid", NULL, &Engine::GetInstance().editor->debugGrid);
			ImGui::MenuItem("Camera", NULL, &Engine::GetInstance().editor->debugCamera);
			ImGui::MenuItem("Mesh", NULL, &Engine::GetInstance().editor->debugMesh);
			ImGui::MenuItem("Normals", NULL, &Engine::GetInstance().editor->debugNormal);
			ImGui::MenuItem("AABB", NULL, &Engine::GetInstance().editor->debugAABB);
			ImGui::MenuItem("Ray", NULL, &Engine::GetInstance().editor->debugRay);
			ImGui::MenuItem("Tree", NULL, &Engine::GetInstance().editor->debugTree);

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
		cam->SetPerspective(cam->GetFov(), viewportSize.x / viewportSize.y, cam->GetNearPlane(), cam->GetFarPlane());
	}

	unsigned int textureID = cam->textureID;

	ImVec2 winPos = ImGui::GetCursorScreenPos();

	ImGui::Image((ImTextureID)(intptr_t)textureID, viewportSize, ImVec2(0, 1), ImVec2(1, 0));

	//PICKING
	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !ImGuizmo::IsOver())
	{
		ImVec2 mousePos = ImGui::GetMousePos();
		int mouseX = (int)(mousePos.x - winPos.x);
		int mouseY = (int)(mousePos.y - winPos.y);

		Engine::GetInstance().editor->TestMouseRay(mouseX, mouseY, (int)viewportSize.x, (int)viewportSize.y);
	}

	const std::vector<GameObject*>& selectedObjects = Engine::GetInstance().editor->GetSelectedGameObjects();

	// GUIZMO
	if (!selectedObjects.empty())
	{
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_DOWN) Engine::GetInstance().editor->currentGizmoOperation = ImGuizmo::TRANSLATE;
		else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_E) == KEY_DOWN) Engine::GetInstance().editor->currentGizmoOperation = ImGuizmo::SCALE;
		else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_R) == KEY_DOWN) Engine::GetInstance().editor->currentGizmoOperation = ImGuizmo::ROTATE;

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
		ImGuizmo::AllowAxisFlip(false);
		ImGuizmo::SetRect(winPos.x, winPos.y, viewportSize.x, viewportSize.y);

		const glm::mat4& viewMatrix = cam->GetViewMatrix();
		const glm::mat4& projectionMatrix = cam->GetProjectionMatrix();

		glm::vec3 centerPos(0.0f);
		int validTransforms = 0;

		for (GameObject* go : selectedObjects)
		{
			Transform* t = (Transform*)go->GetComponent(ComponentType::Transform);
			if (t) {
				centerPos += t->GetGlobalPosition();
				validTransforms++;
			}
		}

		if (validTransforms > 0)
			centerPos /= (float)validTransforms;

		glm::mat4 groupMatrix = glm::translate(glm::mat4(1.0f), centerPos);
		glm::mat4 oldGroupMatrix = groupMatrix;

		ImGuizmo::Manipulate(
			glm::value_ptr(viewMatrix),
			glm::value_ptr(projectionMatrix),
			Engine::GetInstance().editor->currentGizmoOperation,
			ImGuizmo::WORLD,
			glm::value_ptr(groupMatrix)
		);

		static bool wasUsing = false;
		static std::vector<glm::vec3> initialPositions;
		static std::vector<glm::vec3> initialScales;
		static glm::vec3 initialCenterPos;

		if (!wasUsing && ImGuizmo::IsUsing()) {
			initialPositions.clear();
			initialScales.clear();
			initialCenterPos = centerPos;

			for (GameObject* go : selectedObjects) {
				Transform* t = (Transform*)go->GetComponent(ComponentType::Transform);
				if (t) {
					initialPositions.push_back(t->GetGlobalPosition());
					initialScales.push_back(t->GetScale());
				}
			}
			wasUsing = true;
		}

		bool canUseGuizmo = ImGui::IsWindowHovered();
		ImGuizmo::Enable(canUseGuizmo);

		if (canUseGuizmo && ImGuizmo::IsUsing())
		{
			//SCALE
			if (Engine::GetInstance().editor->currentGizmoOperation == ImGuizmo::SCALE)
			{
				glm::vec3 newPos, newEulerRot, newScale;
				ImGuizmo::DecomposeMatrixToComponents(
					glm::value_ptr(groupMatrix),
					glm::value_ptr(newPos),
					glm::value_ptr(newEulerRot),
					glm::value_ptr(newScale)
				);

				if (glm::isnan(newScale.x)) newScale.x = 0.01f;
				if (glm::isnan(newScale.y)) newScale.y = 0.01f;
				if (glm::isnan(newScale.z)) newScale.z = 0.01f;

				newScale = glm::max(newScale, glm::vec3(0.01f));

				int idx = 0;
				for (GameObject* go : selectedObjects)
				{
					Transform* t = (Transform*)go->GetComponent(ComponentType::Transform);
					if (t && idx < initialPositions.size() && idx < initialScales.size())
					{
						glm::vec3 finalScale = initialScales[idx] * newScale;
						if (glm::isnan(finalScale.x)) finalScale.x = 0.01f;
						if (glm::isnan(finalScale.y)) finalScale.y = 0.01f;
						if (glm::isnan(finalScale.z)) finalScale.z = 0.01f;

						finalScale = glm::max(finalScale, glm::vec3(0.001f));
						t->SetScale(finalScale);

						glm::vec3 vectorFromCenter = initialPositions[idx] - initialCenterPos;
						glm::vec3 scaledVector = vectorFromCenter * newScale;
						glm::vec3 newPosition = initialCenterPos + scaledVector;

						glm::mat4 parentInverse = glm::mat4(1.0f);
						if (go->parent) {
							Transform* pT = (Transform*)go->parent->GetComponent(ComponentType::Transform);
							if (pT) parentInverse = glm::inverse(pT->GetGlobalMatrix());
						}
						glm::vec3 localPos = glm::vec3(parentInverse * glm::vec4(newPosition, 1.0f));
						t->SetPosition(localPos);

						idx++;
					}
				}
			}
			//ROTATION AND TRANSFORM
			else
			{
				glm::mat4 deltaMatrix = groupMatrix * glm::inverse(oldGroupMatrix);

				for (GameObject* go : selectedObjects)
				{
					Transform* t = (Transform*)go->GetComponent(ComponentType::Transform);
					if (t)
					{
						glm::mat4 objectGlobal = t->GetGlobalMatrix();
						glm::mat4 newObjectGlobal = deltaMatrix * objectGlobal;

						glm::mat4 parentInverse = glm::mat4(1.0f);
						if (go->parent) {
							Transform* pT = (Transform*)go->parent->GetComponent(ComponentType::Transform);
							if (pT) parentInverse = glm::inverse(pT->GetGlobalMatrix());
						}

						glm::mat4 newLocal = parentInverse * newObjectGlobal;

						glm::vec3 newPos, newEulerRot, newScale;
						ImGuizmo::DecomposeMatrixToComponents(
							glm::value_ptr(newLocal),
							glm::value_ptr(newPos),
							glm::value_ptr(newEulerRot),
							glm::value_ptr(newScale)
						);

						t->SetPosition(newPos);
						t->SetEulerRotation(newEulerRot);
					}
				}
			}
		}
		else
		{
			if (wasUsing) {
				wasUsing = false;
			}
		}
	}

	Engine::GetInstance().editor->GetEditorCamera()->LockCamera(ImGuizmo::IsUsing() || !ImGui::IsWindowHovered());

	ImGui::End();
}