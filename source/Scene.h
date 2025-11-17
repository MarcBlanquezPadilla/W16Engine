#pragma once
#include "Module.h"
#include <vector>

class GameObject;

class Scene : public Module
{
public:

	Scene(bool startEnabled) ;

	virtual ~Scene();

	bool Awake();
	bool Start();

	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();

	bool CleanUp();

	std::vector<GameObject*> GetGameObjects() { return gameObjects; }
	std::vector<GameObject*> GetAllGameObjects();
	void CollectGameObjectsRecursive(GameObject* go, std::vector<GameObject*>& list);

	GameObject* GetSelectedGameObject() { return selectedGameObject; }

	void SetSelectedGameObject(GameObject* gameObject);

	void AddGameObject(GameObject* gameObject);

private:
	std::vector<GameObject*> gameObjects;
	GameObject* selectedGameObject;
};