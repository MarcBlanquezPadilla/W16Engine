#pragma once
#include "Module.h"
#include "components/Component.h"
#include <map>

class GameObject
{
public:

	GameObject(bool _enabled);

	~GameObject();

	bool Awake();

	bool Update(float dt);

	bool CleanUp();

	Component* AddComponent(ComponentType type);
	Component* GetComponent(ComponentType type);

public:
	bool enabled;
	std::map<ComponentType, Component*> components;
};