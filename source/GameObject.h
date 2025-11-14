#pragma once
#include "Module.h"
#include "components/Component.h"
#include <map>
#include <vector>
#include <string>
#include "pugixml.hpp"

class Transform;

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

	void SetSelected(bool selected);

public:
	std::string name;
	bool enabled;
	bool selected;
	
	GameObject* parent;
	std::vector<GameObject*> childs;

	Transform* transform;
	std::map<ComponentType, Component*> components;

	uint32_t UUID;
	uint32_t parentUUID;
	
};