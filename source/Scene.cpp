#include "Scene.h"
#include "GameObject.h"
#include "EventSystem.h"
#include "Engine.h"

#include "utils/Log.h"
#include "utils/Tree.h"
#include "utils/AABB.h"
#include "utils/Ray.h"
#include <list>
#include <cmath>
#include "glm/glm.hpp"

Scene::Scene(bool startEnabled) : Module(startEnabled)
{
	
}

Scene::~Scene()
{

}

bool Scene::Awake()
{
	bool ret = true;
	
	staticTree = new Tree(TreeType::Octree, 6, 8);
	staticTreeDirty = true;

	Engine::GetInstance().events->Subscribe(Event::Type::TransformChanged, this);
	Engine::GetInstance().events->Subscribe(Event::Type::StaticChanged, this);

	return ret;
}

bool Scene::Start()
{
	bool ret = true;

	return ret;
}

bool Scene::PreUpdate()
{
	bool ret = true;

	return ret;
}

bool Scene::Update(float dt)
{
	bool ret = true;

	//GAME OBJECTS
	for each(GameObject* gameObject in gameObjects)
	{
		gameObject->Update(dt);
	}


	staticTree->DrawDebug(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

	return ret;
}

bool Scene::PostUpdate()
{
	bool ret = true;

	return ret;
}

bool Scene::CleanUp()
{
	bool ret = true;

	LOG("Cleaning Scene");
	staticTree->Clear();
	delete staticTree;

	for (int i = 0; i < gameObjects.size(); i++)
	{
		gameObjects[i]->CleanUp();
		delete gameObjects[i];
	}

	gameObjects.clear();

	Engine::GetInstance().events->UnsubscribeAll(this);

	return ret;
}

#pragma region GameObjects

void Scene::AddGameObject(GameObject* gameObject)
{
	std::string baseName = gameObject->name;
	std::string newName = baseName;
	int counter = 1;

	while (true)
	{
		bool nameCollision = false;


		for (GameObject* existingGO : gameObjects)
		{
			if (existingGO->name == newName)
			{
				nameCollision = true;
				break;
			}
		}

		if (!nameCollision)
		{
			break;
		}

		newName = baseName + " (" +std::to_string(counter) + ")";
		counter++;
	}

	gameObject->name = newName;
	gameObjects.push_back(gameObject);
}


void Scene::RemoveGameObject(GameObject* go)
{
	auto it = std::remove(gameObjects.begin(), gameObjects.end(), go);
	if (it != gameObjects.end()) gameObjects.erase(it, gameObjects.end());
}

#pragma endregion

#pragma region Tree

void Scene::RebuildTree()
{
	std::vector<GameObject*> staticObjects;
	std::vector<GameObject*> dynamicObjects;

	for (GameObject* gameObject : GetAllGameObjects())
	{
		if (gameObject->GetStatic()) staticObjects.push_back(gameObject);
		else dynamicObjects.push_back(gameObject);
	}
	
	if (staticTreeDirty)
	{
		staticTree->Build(staticObjects, GetWorldLimits());
		staticTreeDirty = false;
		LOG("Static octree rebuilt with %d objects, %d nodes",
			staticObjects.size(), staticTree->GetNodeCount());
	}
}

#pragma endregion

#pragma region Ray

void Scene::QueryRay(Ray ray, std::vector<GameObject*>& results)
{
	QueryRayToStatic(ray, results);
	QueryRayToDynamic(ray, results);
}

void Scene::QueryRayToStatic(Ray ray, std::vector<GameObject*>& results)
{
	std::vector<GameObject*> staticResults;

	RebuildTree();
	staticTree->QueryRay(ray, staticResults);

	results.insert(results.end(), staticResults.begin(), staticResults.end());
}

void Scene::QueryRayToDynamic(Ray ray, std::vector<GameObject*>& results)
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

void Scene::CollectGameObjectsRecursive(GameObject* go, std::vector<GameObject*>& list)
{
	list.push_back(go);

	for (GameObject* child : go->childs)
	{
		CollectGameObjectsRecursive(child, list);
	}
}

std::vector<GameObject*> Scene::GetAllGameObjects()
{
	std::vector<GameObject*> allGameObjects;

	for (GameObject* go : gameObjects)
	{
		CollectGameObjectsRecursive(go, allGameObjects);
	}

	return allGameObjects;
}


std::vector<GameObject*> Scene::GetDynamicGameObjects()
{
	std::vector<GameObject*> dynamicGameObjects;
	
	for (GameObject* obj : GetAllGameObjects())
	{
		if (obj && !obj->GetStatic())
		{
			dynamicGameObjects.push_back(obj);
		}
	}

	return dynamicGameObjects;
}

std::vector<GameObject*> Scene::GetStaticGameObjects()
{
	std::vector<GameObject*> staticGameObjects;

	for (GameObject* obj : GetAllGameObjects())
	{
		if (obj && obj->GetStatic())
		{
			staticGameObjects.push_back(obj);
		}
	}

	return staticGameObjects;
}

AABB Scene::GetWorldLimits()
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

	return mapLimits;
}

#pragma endregion

void Scene::OnEvent(const Event& event)
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
		{
			GameObject* gameObject = event.data.gameObject.gameObject;
			if (!gameObject) return;
			MarkStaticTreeDirty();
		}
		break;
	}

	default:
		break;
	}
}