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

	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();

	bool CleanUp();

	void Scene::HandleAssetDrop(const std::string& path);

	bool LoadModel(const std::string& filePath);
	bool LoadTexture(const std::string& filePath);
	std::vector<GameObject*>& GetGameObjects() { return gameObjects; }
	GameObject* GetSelectedGameObject() { return selectedGameObject; }
	void SetSelectedGameObject(GameObject* gameObject) { selectedGameObject = gameObject; }

private:
	std::vector<GameObject*> gameObjects;
	GameObject* selectedGameObject;
};