#pragma once
#include "Module.h"
#include "components/Component.h"
#include <map>
#include <vector>
#include <string>
#include "glm/glm.hpp"

class Transform;
class AABB;
class Config;

class GameObject
{
public:

	GameObject(bool _enabled, std::string _name);

	~GameObject();

	bool OnEnable();

	bool OnDisable();

	bool Update(float dt);

	bool CleanUp();

	void AddChild(GameObject* gameObject);
	void RemoveChild(GameObject* childToRemove);
	std::vector<GameObject*> GetChilds() { return childs;}
	void SetParent(GameObject* newParent);

	bool IsDescendant(GameObject* potentialParent);

	void Save(Config& gameObjectNode);
	void Load(Config& gameObjectNode);

	//GETTERS & SETTERS
	void SetStatic(bool s);
	bool GetStatic();
	void SetEnabled(bool enabled);
	void UpdateEnabledRecursive(bool enabled);
	bool GetEnabled();

	//COMPONENTS
	Component* AddComponent(ComponentType type);
	Component* GetComponent(ComponentType type);
	bool TryGetGlobalMatrix(glm::mat4& globalMatrix);
	bool TryGetGlobalAABB(AABB& globalAABB);
	bool TryGetComponent(ComponentType type, Component*& component);

	void Destroy();

public:
	std::string name;

	
	GameObject* parent;
	std::vector<GameObject*> childs;

	Transform* transform;
	std::map<ComponentType, Component*> components;

	uint32_t UUID;
	bool pendingToDelete = false;

private:
	bool enabled;
	bool isStatic;

};