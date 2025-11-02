#include "Scene.h"
#include "GameObject.h"
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

	return ret;
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
	selectedGameObject = gameObject;
}