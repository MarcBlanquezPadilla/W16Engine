#include "ModuleScene.h"
#include "GameObject.h"
#include "ModuleEvents.h"
#include "Engine.h"

#include "utils/Log.h"
#include "utils/Tree.h"
#include "utils/AABB.h"
#include "utils/Ray.h"
#include <list>
#include <cmath>
#include "glm/glm.hpp"

ModuleScene::ModuleScene(bool startEnabled) : Module(startEnabled)
{
	
}

ModuleScene::~ModuleScene()
{

}

bool ModuleScene::Awake()
{
	bool ret = true;
	
	staticTree = new Tree(TreeType::Octree, 6, 8);
	staticTreeDirty = true;

	Engine::GetInstance().moduleEvents->Subscribe(Event::Type::TransformChanged, this);
	Engine::GetInstance().moduleEvents->Subscribe(Event::Type::StaticChanged, this);

	return ret;
}

bool ModuleScene::Start()
{
	bool ret = true;

	return ret;
}

bool ModuleScene::PreUpdate()
{
	bool ret = true;

	return ret;
}

bool ModuleScene::Update()
{
	bool ret = true;

	//GAME OBJECTS
	for (GameObject* gameObject : allGameObjects)
	{
		gameObject->Update();
	}

	return ret;
}

bool ModuleScene::PostUpdate()
{
	if (!objectsPendingToDelete.empty())
	{
		for (GameObject* go : objectsPendingToDelete)
		{
			auto it = std::remove(allGameObjects.begin(), allGameObjects.end(), go);
			if (it != allGameObjects.end()) allGameObjects.erase(it, allGameObjects.end());

			if (go->parent == nullptr)
			{
				auto itR = std::remove(rootGameObjects.begin(), rootGameObjects.end(), go);
				if (itR != rootGameObjects.end()) rootGameObjects.erase(itR, rootGameObjects.end());
			}

			auto itS = std::remove(staticGameObjects.begin(), staticGameObjects.end(), go);
			if (itS != staticGameObjects.end())
			{
				staticGameObjects.erase(itS, staticGameObjects.end());
				MarkStaticTreeDirty();
			}

			auto itD = std::remove(dynamicGameObjects.begin(), dynamicGameObjects.end(), go);
			if (itD != dynamicGameObjects.end())
			{
				dynamicGameObjects.erase(itD, dynamicGameObjects.end());
			}

			if (go->parent != nullptr)
			{
				go->parent->RemoveChild(go);
			}

			go->CleanUp();
			delete go;
		}

		objectsPendingToDelete.clear();
	}

	return true;
}
bool ModuleScene::CleanUp()
{
	bool ret = true;

	LOG("Cleaning Scene");
	Engine::GetInstance().moduleEvents->UnsubscribeAll(this);

	objectsPendingToDelete.clear();

	if (staticTree) {
		staticTree->Clear();
		delete staticTree;
		staticTree = nullptr;
	}

	for (GameObject* go : allGameObjects)
	{
		if (go) {
			go->CleanUp();
			delete go;
		}
	}

	allGameObjects.clear();
	rootGameObjects.clear();
	staticGameObjects.clear();
	dynamicGameObjects.clear();
	objectsPendingToDelete.clear();

	Engine::GetInstance().moduleEvents->UnsubscribeAll(this);

	return ret;
}

bool ModuleScene::NewScene()
{
	bool ret = true;
	LOG("Creating New Scene");

	for (int i = 0; i < allGameObjects.size(); i++)
	{
		allGameObjects[i]->CleanUp();
		delete allGameObjects[i];
	}
	allGameObjects.clear();
	rootGameObjects.clear();
	staticGameObjects.clear();
	dynamicGameObjects.clear();
	objectsPendingToDelete.clear();

	if (staticTree) staticTree->Clear();
	staticTreeDirty = true;

	Engine::GetInstance().moduleEvents->PublishImmediate(Event(Event::Type::SceneCleared));

	return ret;
}

#pragma region GameObjects

void ModuleScene::AddGameObject(GameObject* gameObject)
{
	allGameObjects.push_back(gameObject);

	if (gameObject->GetStatic())
	{
		staticGameObjects.push_back(gameObject);
		MarkStaticTreeDirty();
	}
	else
	{
		dynamicGameObjects.push_back(gameObject);
	}

	if (gameObject->parent == nullptr)
	{
		rootGameObjects.push_back(gameObject);
	}

	for (GameObject* child : gameObject->childs)
	{
		AddGameObject(child);
	}
}


void ModuleScene::RemoveGameObject(GameObject* go)
{
	if (go == nullptr) return;

	for (GameObject* child : go->childs)
	{
		RemoveGameObject(child);
	}

	auto it = std::remove(allGameObjects.begin(), allGameObjects.end(), go);
	if (it != allGameObjects.end()) allGameObjects.erase(it, allGameObjects.end());

	if (go->GetStatic())
	{
		auto itS = std::remove(staticGameObjects.begin(), staticGameObjects.end(), go);
		if (itS != staticGameObjects.end())
		{
			staticGameObjects.erase(itS, staticGameObjects.end());
			MarkStaticTreeDirty();
		}
	}
	else
	{
		auto itD = std::remove(dynamicGameObjects.begin(), dynamicGameObjects.end(), go);
		if (itD != dynamicGameObjects.end())
		{
			dynamicGameObjects.erase(itD, dynamicGameObjects.end());
		}
	}

	auto itR = std::remove(rootGameObjects.begin(), rootGameObjects.end(), go);
	if (itR != rootGameObjects.end()) rootGameObjects.erase(itR, rootGameObjects.end());
}

void ModuleScene::DestroyGameObject(GameObject* gameObject)
{
	if (!gameObject || gameObject->pendingToDelete) return;

	for (GameObject* child : gameObject->childs)
	{
		DestroyGameObject(child);
	}

	gameObject->pendingToDelete = true;
	objectsPendingToDelete.push_back(gameObject);
}

void ModuleScene::AddToRoots(GameObject* go)
{
	rootGameObjects.push_back(go);
}

void ModuleScene::RemoveFromRoots(GameObject* go)
{
	auto it = std::remove(rootGameObjects.begin(), rootGameObjects.end(), go);
	if (it != rootGameObjects.end())
		rootGameObjects.erase(it, rootGameObjects.end());
}

#pragma endregion

#pragma region Tree

void ModuleScene::RebuildTree()
{
	if (staticTreeDirty)
	{
		staticTree->Build(staticGameObjects, GetWorldLimits());

		staticTreeDirty = false;

		LOG("Static octree rebuilt with %d objects", staticGameObjects.size());
	}
}

#pragma endregion

#pragma region Ray

void ModuleScene::QueryRay(Ray ray, std::vector<GameObject*>& results)
{
	QueryRayToStatic(ray, results);
	QueryRayToDynamic(ray, results);
}

void ModuleScene::QueryRayToStatic(Ray ray, std::vector<GameObject*>& results)
{
	std::vector<GameObject*> staticResults;

	RebuildTree();
	staticTree->QueryRay(ray, staticResults);

	results.insert(results.end(), staticResults.begin(), staticResults.end());
}

void ModuleScene::QueryRayToDynamic(Ray ray, std::vector<GameObject*>& results)
{
	std::vector<GameObject*> dynamicResults;

	for (GameObject* obj : GetDynamicGameObjects())
	{
		AABB aabb;
		glm::mat4 globalMatrix;
		if (obj->GetStatic() || !obj->TryGetGlobalAABB(aabb) || !obj->TryGetGlobalMatrix(globalMatrix)) continue;

		float dist;
		if (ray.RayIntersectsAABB(aabb, dist))
		{
			dynamicResults.push_back(obj);
		}
	}

	results.insert(results.end(), dynamicResults.begin(), dynamicResults.end());
}

#pragma endregion

#pragma region Getters

GameObject* ModuleScene::GetObjectByUUID(UID uuid)
{
	for (GameObject* gameObject : GetAllGameObjects())
	{
		if (gameObject->UUID == uuid)
		{
			return gameObject;
		}
	}
	return nullptr;
}

AABB ModuleScene::GetWorldLimits()
{
	AABB mapLimits;
	mapLimits.min = glm::vec3(INFINITY);
	mapLimits.max = glm::vec3(-INFINITY);

	bool hasMeshes = false;

	for (GameObject* gameObject : GetAllGameObjects())
	{
		AABB objectAABB;
		if (!gameObject || !gameObject->TryGetGlobalAABB(objectAABB)) continue;

		hasMeshes = true;
		mapLimits.min = glm::min(mapLimits.min, objectAABB.min);
		mapLimits.max = glm::max(mapLimits.max, objectAABB.max);
	}

	if (!hasMeshes)
	{
		mapLimits.min = glm::vec3(-100.0f);
		mapLimits.max = glm::vec3(100.0f);
	}
	else
	{
		float margin = 1.0f;
		mapLimits.min -= glm::vec3(margin);
		mapLimits.max += glm::vec3(margin);
	}

	return mapLimits;
}

#pragma endregion

void ModuleScene::OnEvent(const Event& event)
{
	switch (event.type)
	{
	case Event::Type::TransformChanged:
	{
		{
			GameObject* gameObject = event.data.gameObject.gameObject;
			if(!gameObject) return;
			if (gameObject->GetStatic()) 
				MarkStaticTreeDirty();
		}
		break;
	}
	case Event::Type::StaticChanged:
	{
		GameObject* gameObject = event.data.gameObject.gameObject;
		if (!gameObject) return;
		if (gameObject->GetStatic())
		{
			auto it = std::remove(dynamicGameObjects.begin(), dynamicGameObjects.end(), gameObject);
			if (it != dynamicGameObjects.end()) dynamicGameObjects.erase(it, dynamicGameObjects.end());

			staticGameObjects.push_back(gameObject);

			MarkStaticTreeDirty();
		}
		else
		{
			auto it = std::remove(staticGameObjects.begin(), staticGameObjects.end(), gameObject);
			if (it != staticGameObjects.end()) staticGameObjects.erase(it, staticGameObjects.end());

			dynamicGameObjects.push_back(gameObject);

			MarkStaticTreeDirty();
		}
		break;
	}

	default:
		break;
	}
}