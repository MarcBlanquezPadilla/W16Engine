#include "Engine.h"
#include "ModuleEditor.h"
#include "ModuleRender.h"
#include "Interface.h"
#include "Global.h"
#include "EditorCamera.h"
#include "CameraLens.h"
#include "ModuleEvents.h"

#include "ModuleInput.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "components/Transform.h"
#include "components/Camera.h"
#include "components/Mesh.h"

#include "utils/Ray.h"
#include "utils/AABB.h"
#include "utils/Tree.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>
#include "ImGuizmo.h"
#include "Imgui.h"
#include <algorithm>

ModuleEditor::ModuleEditor(bool startEnabled) : Module(startEnabled)
{
	name = "Editor";
}

ModuleEditor::~ModuleEditor() {}

bool ModuleEditor::Awake()
{
	bool ret = true;

	//SUBSCRIBE TO INPUT EVENT
	Engine::GetInstance().moduleEvents->Subscribe(Event::Type::EventSDL, this);
	Engine::GetInstance().moduleEvents->Subscribe(Event::Type::CastRay, this);
	Engine::GetInstance().moduleEvents->Subscribe(Event::Type::SceneCleared, this);
	Engine::GetInstance().moduleEvents->Subscribe(Event::Type::GameObjectDestroyed, this);

	//INIT INTERFACE
	userInterface = new Interface();
	userInterface->Awake();

	//INIT EDITOR CAMERA
	editorCamera = new EditorCamera();
	editorCamera->Awake();

	//DEBUG
	startLastRay = { 0,0,0 };
	endLastRay = { 0,0,0 };
	debugRay = false;
	debugTree = false;
	debugMesh = false;
	debugAABB = false;
	debugNormal = false;
	debugGrid = true;
	debugCamera = true;

	gridSize = 10;
	gridRows = 20;
	gridColumns = 20;

	selectedGameObjects.clear();

	return ret;
}

bool ModuleEditor::PreUpdate()
{
	userInterface->PreUpdate();
	editorCamera->PreUpdate();

	return true;
}

bool ModuleEditor::Update(float dt)
{
	if (!selectedGameObjects.empty() && Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_DELETE) == KEY_DOWN)
	{
		for (GameObject* selectedGameObject : selectedGameObjects)
		{
			selectedGameObject->Destroy();
		}
	}

	if (debugRay)
	{
		Engine::GetInstance().moduleRender->DrawLine(startLastRay, endLastRay, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	}

	if (debugTree)
	{
		Tree* tree = Engine::GetInstance().moduleScene->GetTree();
		if (tree)
		{
			std::vector<AABB> allNodesAABB;
			tree->GetAllNodes(allNodesAABB);
			glm::vec4 color = glm::vec4(DEBUG_COLOR);


			ModuleRender* render = Engine::GetInstance().moduleRender;

			for (const AABB& box : allNodesAABB)
			{
				glm::vec3 min = box.min;
				glm::vec3 max = box.max;

				glm::vec3 v0 = min;
				glm::vec3 v1 = glm::vec3(max.x, min.y, min.z);
				glm::vec3 v2 = glm::vec3(max.x, max.y, min.z);
				glm::vec3 v3 = glm::vec3(min.x, max.y, min.z);

				glm::vec3 v4 = glm::vec3(min.x, min.y, max.z);
				glm::vec3 v5 = glm::vec3(max.x, min.y, max.z);
				glm::vec3 v6 = max;
				glm::vec3 v7 = glm::vec3(min.x, max.y, max.z);

				render->DrawLine(v0, v1, color);
				render->DrawLine(v1, v2, color);
				render->DrawLine(v2, v3, color);
				render->DrawLine(v3, v0, color);

				render->DrawLine(v4, v5, color);
				render->DrawLine(v5, v6, color);
				render->DrawLine(v6, v7, color);
				render->DrawLine(v7, v4, color);

				render->DrawLine(v0, v4, color);
				render->DrawLine(v1, v5, color);
				render->DrawLine(v2, v6, color);
				render->DrawLine(v3, v7, color);
			}
		}
	}

	if (debugGrid)
	{
		float startX = -gridRows * gridSize / 2;
		float startZ = -gridColumns * gridSize / 2;

		int currentX;
		int currentZ;
		
		ModuleRender* render = Engine::GetInstance().moduleRender;

		glm::vec4 gridColor = glm::vec4(GRID_COLOR);

		for (int x = 0; x <= gridRows; x++)
		{	
			currentX = startX + x * gridSize;
			glm::vec3 startPos = glm::vec3(currentX, 0, startZ);
			glm::vec3 endPos = glm::vec3(currentX, 0, -startZ);
			render->DrawLine(startPos, endPos, gridColor);
		}

		for (int z = 0; z <= gridColumns; z++)
		{
			currentZ = startZ + z * gridSize;
			glm::vec3 startPos = glm::vec3(startX, 0, currentZ);
			glm::vec3 endPos = glm::vec3(-startX, 0, currentZ);
			render->DrawLine(startPos, endPos, gridColor);
		}
	}

	
	if (!selectedGameObjects.empty())
	{
		for (GameObject* selectedGameObject : selectedGameObjects)
		{
			Mesh* selectedMesh = nullptr;
			selectedMesh = (Mesh*)selectedGameObject->GetComponent(ComponentType::Mesh);

			if (selectedMesh)
			{
				//DEBUG MESH
				selectedMesh->drawMesh = debugMesh;

				//DEBUG NORMALS
				selectedMesh->drawNormals = debugNormal;

				//DEBUG AABB
				if (debugAABB)
				{
					Transform* selectedTransform = selectedGameObject->transform;

					if (selectedTransform)
					{
						glm::vec3 localMin = selectedMesh->aabb->min;
						glm::vec3 localMax = selectedMesh->aabb->max;

						glm::vec3 localCorners[8] = {
							{ localMin.x, localMin.y, localMin.z },
							{ localMax.x, localMin.y, localMin.z },
							{ localMin.x, localMax.y, localMin.z },
							{ localMax.x, localMax.y, localMin.z },
							{ localMin.x, localMin.y, localMax.z },
							{ localMax.x, localMin.y, localMax.z },
							{ localMin.x, localMax.y, localMax.z },
							{ localMax.x, localMax.y, localMax.z }
						};

						glm::mat4 modelMatrix = selectedTransform->GetGlobalMatrix();

						glm::vec3 globalMin = glm::vec3(FLT_MAX);
						glm::vec3 globalMax = glm::vec3(-FLT_MAX);

						for (int i = 0; i < 8; i++)
						{
							glm::vec4 transformed = modelMatrix * glm::vec4(localCorners[i], 1.0f);
							glm::vec3 worldPos = glm::vec3(transformed);

							globalMin = glm::min(globalMin, worldPos);
							globalMax = glm::max(globalMax, worldPos);
						}

						ModuleRender* render = Engine::GetInstance().moduleRender;
						glm::vec4 color = glm::vec4(DEBUG_COLOR);

						glm::vec3 p1 = globalMin;
						glm::vec3 p2 = glm::vec3(globalMax.x, globalMin.y, globalMin.z);
						glm::vec3 p3 = glm::vec3(globalMin.x, globalMax.y, globalMin.z);
						glm::vec3 p4 = glm::vec3(globalMax.x, globalMax.y, globalMin.z);

						glm::vec3 p5 = glm::vec3(globalMin.x, globalMin.y, globalMax.z);
						glm::vec3 p6 = glm::vec3(globalMax.x, globalMin.y, globalMax.z);
						glm::vec3 p7 = glm::vec3(globalMin.x, globalMax.y, globalMax.z);
						glm::vec3 p8 = globalMax;

						render->DrawLine(p1, p2, color);
						render->DrawLine(p2, p4, color);
						render->DrawLine(p4, p3, color);
						render->DrawLine(p3, p1, color);

						render->DrawLine(p5, p6, color);
						render->DrawLine(p6, p8, color);
						render->DrawLine(p8, p7, color);
						render->DrawLine(p7, p5, color);

						render->DrawLine(p1, p5, color);
						render->DrawLine(p2, p6, color);
						render->DrawLine(p3, p7, color);
						render->DrawLine(p4, p8, color);
					}
				}
			}
			if (debugCamera)
			{
				Camera* selectedCamera = nullptr;
				selectedCamera = (Camera*)selectedGameObject->GetComponent(ComponentType::Camera);
				if (selectedCamera)
				{
					glm::vec4 corners[8] = {
					{-1, -1, -1, 1}, { 1, -1, -1, 1}, { 1,  1, -1, 1}, {-1,  1, -1, 1},
					{-1, -1,  1, 1}, { 1, -1,  1, 1}, { 1,  1,  1, 1}, {-1,  1,  1, 1}
					};

					glm::mat4 inverseViewProj = glm::inverse(selectedCamera->GetLens()->GetProjectionMatrix() * selectedCamera->GetLens()->GetViewMatrix());

					for (int i = 0; i < 8; ++i)
					{
						corners[i] = inverseViewProj * corners[i];
						corners[i] /= corners[i].w;
					}

					glm::vec4 color = glm::vec4(CAMERA_COLOR);
					ModuleRender* render = Engine::GetInstance().moduleRender;

					render->DrawLine(corners[0], corners[1], color);
					render->DrawLine(corners[1], corners[2], color);
					render->DrawLine(corners[2], corners[3], color);
					render->DrawLine(corners[3], corners[0], color);

					render->DrawLine(corners[4], corners[5], color);
					render->DrawLine(corners[5], corners[6], color);
					render->DrawLine(corners[6], corners[7], color);
					render->DrawLine(corners[7], corners[4], color);

					render->DrawLine(corners[0], corners[4], color);
					render->DrawLine(corners[1], corners[5], color);
					render->DrawLine(corners[2], corners[6], color);
					render->DrawLine(corners[3], corners[7], color);
				}
			}
		}
	}

	//INTERFACE
	userInterface->Update(dt);
	return true;
}

bool ModuleEditor::PostUpdate()
{
	userInterface->PostUpdate();

	return true;
}

bool ModuleEditor::CleanUp()
{

	Engine::GetInstance().moduleEvents->UnsubscribeAll(this);

	userInterface->CleanUp();
	editorCamera->CleanUp();
	
	delete editorCamera;
	delete userInterface;

	return true;
}

void ModuleEditor::TestMouseRay(int mouseX, int mouseY, int width, int height)
{
	Ray ray = editorCamera->GetCameraLens()->GetRayFromMouse(mouseX, mouseY, width, height);

	std::vector <GameObject*> candidates;

	Engine::GetInstance().moduleScene->QueryRay(ray, candidates);

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

	bool ctrlPressed = Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT;
	bool shiftPressed = Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT;
	bool eraseSelecteds = !(ctrlPressed || shiftPressed);

	if (closestHit) {
		SetSelected(closestHit, eraseSelecteds);
	}
	else SetSelected(nullptr, eraseSelecteds);
}

void ModuleEditor::SetSelected(GameObject* gameObject, bool eraseSelecteds)
{

	if (eraseSelecteds && gameObject != nullptr)
	{
		for (GameObject* go : selectedGameObjects) {
			Mesh* mesh = (Mesh*)go->GetComponent(ComponentType::Mesh);
			if (mesh) { mesh->drawStencil = false; mesh->drawNormals = false; mesh->drawMesh = false; }
		}
		selectedGameObjects.clear();
	}
	if (gameObject == nullptr)
	{
		if (eraseSelecteds) {

		}
		return;
	}

	auto it = std::find(selectedGameObjects.begin(), selectedGameObjects.end(), gameObject);

	if (it != selectedGameObjects.end())
	{
		if (!eraseSelecteds) {
			Mesh* mesh = (Mesh*)gameObject->GetComponent(ComponentType::Mesh);
			if (mesh) mesh->drawStencil = false;
			selectedGameObjects.erase(it);
		}
	}
	else
	{
		selectedGameObjects.push_back(gameObject);
		Mesh* mesh = (Mesh*)gameObject->GetComponent(ComponentType::Mesh);
		if (mesh) mesh->drawStencil = true;
	}
}

void ModuleEditor::HandleInput(SDL_Event* event)
{
	userInterface->HandleInput(event);
}

EditorCamera* ModuleEditor::GetEditorCamera()
{
	return editorCamera;
}
CameraLens* ModuleEditor::GetEditorCameraLens()
{
	return editorCamera->GetCameraLens();
}

void ModuleEditor::OnEvent(const Event& event)
{
	switch (event.type)
	{
		case Event::Type::EventSDL:
		{
			HandleInput(event.data.event.event);
			break;
		}
		case Event::Type::CastRay:
		{
			startLastRay = event.data.ray.ray->origin;
			endLastRay = event.data.ray.ray->origin + (event.data.ray.ray->direction * 100.0f);
			break;
		}
		case Event::Type::SceneCleared:
		{
			selectedGameObjects.clear();
			break;
		}
		case Event::Type::GameObjectDestroyed:
		{
			GameObject* destroyedGO = event.data.gameObject.gameObject;

			auto it = std::remove(selectedGameObjects.begin(), selectedGameObjects.end(), destroyedGO);

			if (it != selectedGameObjects.end())
			{
				selectedGameObjects.erase(it, selectedGameObjects.end());
			}
			break;
		}
		default:
			break;
	}
}