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
	const std::vector<GameObject*>& GetGameObjects() const { return gameObjects; }

public:
	std::vector<GameObject*> gameObjects;
};