#include "Engine.h"
#include "Editor.h"
#include "Render.h"
#include "Interface.h"
#include "EditorCamera.h"
#include "CameraLens.h"
#include "EventSystem.h"

#include "Input.h"
#include "Scene.h"
#include "GameObject.h"
#include "components/Transform.h"
#include "components/Mesh.h"

#include "utils/Ray.h"
#include "utils/AABB.h"
#include "utils/Tree.h"

#define GLM_ENABLE_EXPERIMENTAL
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
	Engine::GetInstance().events->Subscribe(Event::Type::EventSDL, this);

	//INIT INTERFACE
	userInterface = new Interface();
	userInterface->Awake();

	//INIT EDITOR CAMERA
	editorCamera = new EditorCamera();
	editorCamera->Awake();

	//DEBUG
	startLastRay = { 0,0,0 };
	endLastRay = { 0,0,0 };
	debugRay = true;
	debugMesh = true;
	debugAABB = false;

	selectedGameObject = nullptr;

	return ret;
}

bool Editor::PreUpdate()
{
	userInterface->PreUpdate();
	editorCamera->PreUpdate();

	return true;
}

bool Editor::Update(float dt)
{
	if (debugRay)
	{
		Engine::GetInstance().render->DrawLine(startLastRay, endLastRay, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	}

	if (debugMesh)
	{
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

	//INTERFACE
	userInterface->Update(dt);
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
	editorCamera->CleanUp();
	
	delete editorCamera;
	delete userInterface;

	return true;
}

void Editor::TestMouseRay(int mouseX, int mouseY, int width, int height)
{
	Ray ray = editorCamera->GetCameraLens()->GetRayFromMouse(mouseX, mouseY, width, height);

	startLastRay = ray.origin;
	endLastRay = ray.origin + (ray.direction * 100.0f);

	std::vector <GameObject*> candidates;

	Engine::GetInstance().scene->QueryRay(ray, candidates);

	GameObject* closestHit = nullptr;
	float minDistance = FLT_MAX;

	for (GameObject* gameObject : candidates)
	{
		GameObject* go = gameObject;
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
	}

	if (closestHit) {
		SetSelected(closestHit);
	}	
}

void Editor::SetSelected(GameObject* gameObject)
{
	if (selectedGameObject)
	{
		Component* meshComp = nullptr;
		if (selectedGameObject->TryGetComponent(ComponentType::Mesh, meshComp))
		{
			Mesh* mesh = static_cast<Mesh*>(meshComp);
			mesh->drawStencil = false;
		}
	}

	if (gameObject)
	{
		selectedGameObject = gameObject;
		Component* meshComp = nullptr;
		if (gameObject->TryGetComponent(ComponentType::Mesh, meshComp))
		{
			Mesh* mesh = static_cast<Mesh*>(meshComp);
			mesh->drawStencil = true;
		}
	}
}

void Editor::HandleInput(SDL_Event* event)
{
	userInterface->HandleInput(event);
}

EditorCamera* Editor::GetEditorCamera()
{
	return editorCamera;
}
CameraLens* Editor::GetEditorCameraLens()
{
	return editorCamera->GetCameraLens();
}

void Editor::OnEvent(const Event& event)
{
	switch (event.type)
	{
	case Event::Type::EventSDL:
	{
		{
			HandleInput(event.data.event.event);
		}
		break;
	}
	default:
		break;
	}
}