#pragma once
#include "Module.h"
#include "components/Component.h"
#include "components/Transform.h"
#include <map>
#include <list>
#include <string>
#include "pugixml.hpp"


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

	void Save(pugi::xml_node gameObjectNode);
	void Load(pugi::xml_node gameObjectNode);

public:
	std::string name;
	bool enabled;
	
	GameObject* parent;
	std::list<GameObject*> childs;

	Transform* transform;
	std::map<ComponentType, Component*> components;

	uint32_t UUID;
	uint32_t parentUUID;
private:
	
};