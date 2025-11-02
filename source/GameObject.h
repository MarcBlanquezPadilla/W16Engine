#pragma once
#include "Module.h"
#include "components/Component.h"
#include <map>
#include <list>
#include <string>

class GameObject
{
public:

	GameObject(bool _enabled, std::string _name);

	~GameObject();

	bool Awake();

	bool Update(float dt);

	bool CleanUp();

	Component* AddComponent(ComponentType type);
	Component* GetComponent(ComponentType type);

	void AddChild(GameObject* gameObject);

public:
	std::string name;
	bool enabled;
	
	GameObject* parent;
	std::list<GameObject*> childs;

	std::map<ComponentType, Component*> components;
};