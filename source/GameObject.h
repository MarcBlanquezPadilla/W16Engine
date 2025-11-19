#pragma once
#include "Module.h"
#include "components/Component.h"
#include <map>
#include <vector>
#include <string>
#include "pugixml.hpp"
#include "glm/glm.hpp"

class Transform;
class AABB;

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
	std::vector<GameObject*> GetChilds() { return childs;}

	void Save(pugi::xml_node gameObjectNode);
	void Load(pugi::xml_node gameObjectNode);

	//GETTERS & SETTERS
	void SetSelected(bool selected);
	bool GetSelected();
	void SetStatic(bool s);
	bool GetStatic();
	void SetEnabled(bool enabled);
	bool GetEnabled();

	//COMPONENTS
	bool TryGetGlobalMatrix(glm::mat4& globalMatrix);
	bool TryGetGlobalAABB(AABB& globalAABB);

public:
	std::string name;

	
	GameObject* parent;
	std::vector<GameObject*> childs;

	Transform* transform;
	std::map<ComponentType, Component*> components;

	uint32_t UUID;
	uint32_t parentUUID;

private:
	bool enabled;
	bool isStatic;
	bool selected;
};