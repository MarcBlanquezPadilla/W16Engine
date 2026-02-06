#pragma once
#include "Module.h"
#include "components/Component.h"
#include <map>
#include <list>
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

	bool Update();
	bool FixedUpdate();

	bool CleanUp();
	bool CleanUpRecursive();



	void AddChild(GameObject* gameObject);
	void RemoveChild(GameObject* childToRemove);
	std::vector<GameObject*> GetChilds() { return childs;}
	void SetParent(GameObject* newParent);
	bool IsDescendant(GameObject* potentialParent);
	GameObject* FindChild(const std::string& findName);

	//SAVE & LOAD
	void Save(Config& gameObjectNode);
	void Load(Config& gameObjectNode);

	//GETTERS & SETTERS
	void SetStatic(bool s);
	const bool& GetStatic();
	void SetEnabled(bool enabled);
	void UpdateEnabledRecursive(bool enabled);
	const bool& GetEnabled();

	//COMPONENTS
	Component* AddComponent(ComponentType type);
	Component* GetComponent(ComponentType type);
	Component* GetComponentInChildren(ComponentType type);
	void GetComponentsInChildren(ComponentType type, std::vector<Component*>& outlist);
	Component* GetComponentInParent(ComponentType type);
	void GetComponentsInParent(ComponentType type, std::vector<Component*>& outlist);
	bool TryGetComponent(ComponentType type, Component*& component);
	void RemoveComponent(ComponentType type);
	void DeletePendingComponents();
	void OnComponentAdded(Component* component);
	void OnComponentRemoved(Component* component);

	bool GetGlobalMatrix(glm::mat4& globalMatrix);
	bool TryGetGlobalAABB(AABB& globalAABB);

	void Destroy();

	void OnEditor();

public:
	std::string name;

	
	GameObject* parent;
	std::vector<GameObject*> childs;

	Transform* transform;
	std::map<ComponentType, Component*> components;
	std::vector<Component*> componentsToDestroy;

	uint32_t UUID;
	bool pendingToDelete = false;

private:
	bool enabled;
	bool isStatic;

};