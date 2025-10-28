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

public:
	std::vector<GameObject*> gameObjects;
};