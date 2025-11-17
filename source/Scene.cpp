#include "Scene.h"
#include "GameObject.h"
#include "utils/Log.h"
#include <list>

Scene::Scene(bool startEnabled) : Module(startEnabled)
{

}

Scene::~Scene()
{

}

bool Scene::Awake()
{
	bool ret = true;
	selectedGameObject = nullptr;
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

	for each(GameObject* gameObject in gameObjects)
	{
		gameObject->Update(dt);
	}

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

	for (int i = 0; i < gameObjects.size(); i++)
	{
		gameObjects[i]->CleanUp();
		delete gameObjects[i];
	}
	gameObjects.clear();

	selectedGameObject = nullptr;

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