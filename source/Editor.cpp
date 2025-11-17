#include "Editor.h"
#include "Engine.h"
#include "Render.h"
#include "Interface.h"
#include "Input.h"
#include "Camera.h"

#include "Scene.h"
#include "GameObject.h"
#include "components/Transform.h"
#include "components/Mesh.h"

#include "utils/Ray.h"
#include "utils/AABB.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtx/intersect.hpp>
#include "ImGuizmo.h"
#include "Imgui.h"
#include <algorithm>

Editor::Editor(bool startEnabled) : Module(startEnabled)
{
	name = "Editor";
}

Editor::~Editor() {}

bool Editor::Awake()
{
	bool ret = true;

	//SUBSCRIBE TO INPUT EVENT
	Engine::GetInstance().input->SubscribeToEvent([this](SDL_Event* event) {
		this->HandleInput(event);
	});

	//INIT INTERFACE
	userInterface = new Interface();
	userInterface->Awake();

	//DEBUG
	startLastRay = { 0,0,0 };
	endLastRay = { 0,0,0 };

	debugRay = false;
	debugMesh = false;

	return ret;
}

bool Editor::PreUpdate()
{
	userInterface->PreUpdate();

	return true;
}

bool Editor::Update(float dt)
{
	//INTERFACE
	userInterface->Update(dt);

	//GUIZMO
	GameObject* selectedGameObject = Engine::GetInstance().scene->GetSelectedGameObject();
	Camera* camera = Engine::GetInstance().camera;

	if (selectedGameObject != nullptr)
	{
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_DOWN) currentGizmoOperation = ImGuizmo::TRANSLATE;
		else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_E) == KEY_DOWN) currentGizmoOperation = ImGuizmo::SCALE;
		else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_R) == KEY_DOWN) currentGizmoOperation = ImGuizmo::ROTATE;

		Transform* transform = (Transform*)selectedGameObject->GetComponent(ComponentType::Transform);
		if (transform)
		{
			ImGuizmo::SetOrthographic(false);

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGuizmo::SetRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);

			const glm::mat4& viewMatrix = camera->GetViewMatrix();
			const glm::mat4& projectionMatrix = camera->GetProjectionMatrix();
			glm::mat4 modelMatrix = transform->GetLocalMatrix();

			ImGuizmo::Manipulate(
				glm::value_ptr(viewMatrix),
				glm::value_ptr(projectionMatrix),
				currentGizmoOperation,
				ImGuizmo::LOCAL,
				glm::value_ptr(modelMatrix)
			);

			bool isGuizmoUsing = ImGuizmo::IsUsing();
			camera->LockCamera(isGuizmoUsing);
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

	//MOUSE PICKING
	if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		if (!ImGui::GetIO().WantCaptureMouse && !ImGuizmo::IsOver())
		{
			int mouseX, mouseY;
			// Necesitas la posición absoluta del ratón en la ventana
			// SDL_GetMouseState(&mouseX, &mouseY); (En Input.cpp puedes hacer un getter para esto)
			Vector2D mousePos = Engine::GetInstance().input->GetMousePosition();

			TestMouseRay(mousePos.getX(), mousePos.getY());
		}
	}

	if (debugRay)
	{
		Engine::GetInstance().render->DrawLine(startLastRay, endLastRay, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	}

	if (debugMesh)
	{
		GameObject* selectedGameObject = Engine::GetInstance().scene->GetSelectedGameObject();
		Mesh* selectedMesh = nullptr;
		if (selectedGameObject)
		{
			Mesh* selectedMesh = (Mesh*)selectedGameObject->GetComponent(ComponentType::Mesh);
			Transform* selectedTransform = selectedGameObject->transform;
			if (selectedMesh && selectedTransform)
			{
				const std::vector<Vertex>& vertices = selectedMesh->GetVertices();
				const std::vector<unsigned int>& indices = selectedMesh->GetIndices();
				glm::mat4 modelMatrix = selectedTransform->GetGlobalMatrix();

				for (size_t i = 0; i < indices.size(); i += 3)
				{
					glm::vec3 v1_local = vertices[indices[i]].position;
					glm::vec3 v2_local = vertices[indices[i + 1]].position;
					glm::vec3 v3_local = vertices[indices[i + 2]].position;

					glm::vec3 v1_world = glm::vec3(modelMatrix * glm::vec4(v1_local, 1.0f));
					glm::vec3 v2_world = glm::vec3(modelMatrix * glm::vec4(v2_local, 1.0f));
					glm::vec3 v3_world = glm::vec3(modelMatrix * glm::vec4(v3_local, 1.0f));

					Render* render = Engine::GetInstance().render;
					glm::vec4 color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

					render->DrawLine(v1_world, v2_world, color);
					render->DrawLine(v2_world, v3_world, color);
					render->DrawLine(v3_world, v1_world, color);
				}
			}
		}
	}
	return true;
}

bool Editor::PostUpdate()
{
	userInterface->PostUpdate();

	return true;
}

bool Editor::CleanUp()
{
	userInterface->CleanUp();
	delete userInterface;

	return true;
}



void Editor::TestMouseRay(int mouseX, int mouseY)
{
	Ray ray = Engine::GetInstance().camera->GetRayFromMouse(mouseX, mouseY);

	startLastRay = ray.origin;
	endLastRay = ray.origin + (ray.direction * 100.0f);

	std::vector<GameObject*> gameObjects = Engine::GetInstance().scene->GetAllGameObjects();
	std::map<float, GameObject*> candidates;

	for (GameObject* go : gameObjects)
	{
		if (!go->enabled) continue;

		Mesh* mesh = (Mesh*)go->GetComponent(ComponentType::Mesh);
		Transform* transform = (Transform*)go->GetComponent(ComponentType::Transform);

		if (mesh && transform)
		{
			AABB globalAABB = mesh->aabb->GetGlobalAABB(transform->GetGlobalMatrix());
			float distance;
			if (ray.RayIntersectsAABB(globalAABB, distance))
			{
				candidates[distance] = go;
			}
		}
	}

	GameObject* closestHit = nullptr;
	float minDistance = FLT_MAX;

	for (auto const& pair : candidates)
	{
		GameObject* go = pair.second;
		Mesh* mesh = (Mesh*)go->GetComponent(ComponentType::Mesh);
		Transform* transform = (Transform*)go->GetComponent(ComponentType::Transform);

		glm::mat4 modelMatrix = transform->GetGlobalMatrix();
		glm::mat4 inverseModel = glm::inverse(modelMatrix);

		Ray localRay;
		localRay.origin = glm::vec3(inverseModel * glm::vec4(ray.origin, 1.0f));
		localRay.direction = glm::normalize(glm::vec3(inverseModel * glm::vec4(ray.direction, 0.0f)));

		const auto& vertices = mesh->GetVertices();
		const auto& indices = mesh->GetIndices();

		for (size_t i = 0; i < indices.size(); i += 3)
		{
			glm::vec3 v0 = vertices[indices[i]].position;
			glm::vec3 v1 = vertices[indices[i + 1]].position;
			glm::vec3 v2 = vertices[indices[i + 2]].position;

			glm::vec2 baryPosition;
			float distance;

			if (glm::intersectRayTriangle(localRay.origin, localRay.direction, v0, v1, v2, baryPosition, distance))
			{
				glm::vec3 localHitPoint = localRay.origin + localRay.direction * distance;
				glm::vec3 worldHitPoint = glm::vec3(modelMatrix * glm::vec4(localHitPoint, 1.0f));
				float worldDistance = glm::distance(ray.origin, worldHitPoint);

				if (worldDistance < minDistance)
				{
					minDistance = worldDistance;
					closestHit = go;
				}
			}
		}

		if (closestHit == go) {
			break;
		}
	}

	if (closestHit) {
		Engine::GetInstance().scene->SetSelectedGameObject(closestHit);
	}
}

void Editor::HandleInput(SDL_Event* event)
{
	userInterface->HandleInput(event);
}
