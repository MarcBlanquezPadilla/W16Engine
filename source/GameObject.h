#pragma once
#include "Module.h"
#include "components/Component.h"
#include <vector>

class GameObject
{
public:

	GameObject(bool _enabled);

	~GameObject();

	bool Awake();

	bool Update(float dt);

	bool CleanUp();

	Component* AddComponent(ComponentType type);

public:
	bool enabled;
	std::vector<Component*> components;
};