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
	for (GameObject* gameObject : gameObjects)
	{
		gameObject->Update(dt);
	}

	return ret;
}

bool Scene::PostUpdate()
{
	//DESTROY GAMEOBJECTS
	if (!objectsPendingToDelete.empty())
	{
		for (GameObject* go : objectsPendingToDelete)
		{

			//LIMPIAR DE LA LISTA PRINCIPAL
			auto it = std::remove(gameObjects.begin(), gameObjects.end(), go);
			if (it != gameObjects.end()) gameObjects.erase(it, gameObjects.end());

			//LIMPIAR DE LISTAS OPTIMIZADAS
			if (go->GetStatic())
			{
				auto itS = std::remove(staticGameObjects.begin(), staticGameObjects.end(), go);
				if (itS != staticGameObjects.end()) {
					staticGameObjects.erase(itS, staticGameObjects.end());
					MarkStaticTreeDirty(); // Solo marcamos dirty, no reconstruimos aquí
				}
			}
			else
			{
				auto itD = std::remove(dynamicGameObjects.begin(), dynamicGameObjects.end(), go);
				if (itD != dynamicGameObjects.end()) dynamicGameObjects.erase(itD, dynamicGameObjects.end());
			}

			//DESVINCULAR DE LA FAMILIA (Para que el padre no tenga un puntero muerto)
			if (go->parent != nullptr)
			{
				go->parent->RemoveChild(go);
			}

			//MUERTE FINAL
			go->CleanUp(); // Lanza evento Destroyed
			delete go;
		}

		// Limpiar la cola
		objectsPendingToDelete.clear();
	}

	return true;

}

bool Scene::CleanUp()
{
	bool ret = true;

	LOG("Cleaning Scene");
	Engine::GetInstance().events->UnsubscribeAll(this);
	// 1. ¡CRÍTICO! Limpiar la cola de pendientes para evitar doble borrado
	objectsPendingToDelete.clear();

	// 2. Limpiar el árbol
	if (staticTree) {
		staticTree->Clear();
		delete staticTree;
		staticTree = nullptr;
	}

	// 3. Borrar todos los GameObjects (Dueño de la memoria)
	for (int i = 0; i < gameObjects.size(); i++)
	{
		if (gameObjects[i])
		{
			gameObjects[i]->CleanUp(); // Emite evento Destroyed
			delete gameObjects[i];
			gameObjects[i] = nullptr;
		}
	}
	gameObjects.clear();

	// 4. ¡CRÍTICO! Limpiar las listas de optimización
	// (Ahora contienen punteros a basura, hay que vaciarlas)
	staticGameObjects.clear();
	dynamicGameObjects.clear();

	Engine::GetInstance().events->UnsubscribeAll(this);

	return ret;
}

bool Scene::NewScene()
{
	bool ret = true;
	LOG("Creating New Scene");

	// 1. Limpiar listas auxiliares
	objectsPendingToDelete.clear();
	staticGameObjects.clear();
	dynamicGameObjects.clear();

	// 2. Borrar objetos
	for (int i = 0; i < gameObjects.size(); i++)
	{
		gameObjects[i]->CleanUp();
		delete gameObjects[i];
	}
	gameObjects.clear();

	// 3. Resetear árbol
	if (staticTree) staticTree->Clear();
	staticTreeDirty = true;

	Engine::GetInstance().events->PublishImmediate(Event(Event::Type::SceneCleared));

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
	if (gameObject->GetStatic())
		staticGameObjects.push_back(gameObject);
	else
		dynamicGameObjects.push_back(gameObject);
}


void Scene::RemoveGameObject(GameObject* go)
{
	auto it = std::remove(gameObjects.begin(), gameObjects.end(), go);
	if (it != gameObjects.end()) gameObjects.erase(it, gameObjects.end());
}

void Scene::DestroyGameObject(GameObject* gameObject)
{
	if (!gameObject || gameObject->pendingToDelete) return;

	gameObject->pendingToDelete = true;
	objectsPendingToDelete.push_back(gameObject);

	for (GameObject* child : gameObject->childs)
	{
		DestroyGameObject(child);
	}
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