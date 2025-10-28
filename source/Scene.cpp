#include<list>
#include "Scene.h"
#include "GameObject.h"
#include "components/Component.h"


Scene::Scene(bool startEnabled) : Module(startEnabled)
{

}

Scene::~Scene()
{

}

bool Scene::Awake()
{
	bool ret = true;
	GameObject* meshObj = new GameObject(true);
	meshObj->AddComponent(ComponentType::Mesh);
	gameObjects.push_back(meshObj);
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