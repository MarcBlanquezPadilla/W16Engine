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
	dynamicTree = new Tree(TreeType::Octree, 6, 8);
	dynamicTreeDirty = true;

	selectedGameObject = nullptr;

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
	dynamicTree->DrawDebug(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

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
	dynamicTree->Clear();
	delete dynamicTree;

	for (int i = 0; i < gameObjects.size(); i++)
	{
		gameObjects[i]->CleanUp();
		delete gameObjects[i];
	}

	gameObjects.clear();

	selectedGameObject = nullptr;

	Engine::GetInstance().events->UnsubscribeAll(this);

	return ret;
}

void Scene::SetSelectedGameObject(GameObject* gameObject)
{
	if (selectedGameObject) selectedGameObject->SetSelected(false);
	selectedGameObject = gameObject;
	selectedGameObject->SetSelected(true);
}


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
	SetSelectedGameObject(gameObject);
}

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

void Scene::RebuildTrees()
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
	if (dynamicTreeDirty)
	{
		dynamicTree->Build(dynamicObjects, GetWorldLimits());
		dynamicTreeDirty = false;
		LOG("Dynamic octree rebuilt with %d objects, %d nodes",
			dynamicObjects.size(), dynamicTree->GetNodeCount());
	}
}

void Scene::QueryRay(Ray ray, std::vector<GameObject*>& results)
{
	results.clear();
	std::vector<GameObject*> staticResults;
	std::vector<GameObject*> dynamicResults;

	RebuildTrees();

	staticTree->QueryRay(ray, staticResults);
	dynamicTree->QueryRay(ray, dynamicResults);

	// Combinar resultados
	results.insert(results.end(), staticResults.begin(), staticResults.end());
	results.insert(results.end(), dynamicResults.begin(), dynamicResults.end());
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

void Scene::OnEvent(const Event& event)
{
	switch (event.type)
	{
	case Event::Type::TransformChanged:
	{
		{
			const TransformChangedEvent& transformChangedEvent = (const TransformChangedEvent&)event;
			if (transformChangedEvent.gameObject->GetStatic()) MarkStaticTreeDirty();
			else MarkDinamicTreeDirty();
		}
		break;
	}

	case Event::Type::StaticChanged:
	{
		{
			const StaticChangedEvent& staticChangedEvent = (const StaticChangedEvent&)event;
			MarkDinamicTreeDirty();
			MarkStaticTreeDirty();
		}
		break;
	}

	case Event::Type::GameObjectCreated:
	{
		{
			const GameObjectEvent& gameObjectEvent = (const GameObjectEvent&)event;
			if (gameObjectEvent.gameObject->GetStatic()) MarkStaticTreeDirty();
			else MarkDinamicTreeDirty();
		}
		break;
	}

	default:
		break;
	}
}