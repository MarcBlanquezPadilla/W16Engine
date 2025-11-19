#pragma once
#include "Module.h"
#include "EventListener.h"
#include <vector>

class GameObject;
class Tree;
class AABB;
struct Ray;

class Scene : public Module, public EventListener
{
public:

	Scene(bool startEnabled) ;

	virtual ~Scene();

	bool Awake();
	bool Start();

	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();

	bool CleanUp();

	//GAMEOBJECT
	std::vector<GameObject*> GetGameObjects() { return gameObjects; }
	std::vector<GameObject*> GetAllGameObjects();
	GameObject* GetSelectedGameObject() { return selectedGameObject; }
	void CollectGameObjectsRecursive(GameObject* go, std::vector<GameObject*>& list);
	void SetSelectedGameObject(GameObject* gameObject);
	void AddGameObject(GameObject* gameObject);

	//WORLD
	AABB GetWorldLimits();

	//TREE
	void RebuildTrees();
	void MarkStaticTreeDirty() { staticTreeDirty = true; }
	void MarkDinamicTreeDirty() { dynamicTreeDirty = true; }
	void QueryRay(Ray ray, std::vector<GameObject*>& results);

	//EVENTS
	void OnEvent(const Event& event) override;

private:
	std::vector<GameObject*> gameObjects;
	GameObject* selectedGameObject;

	Tree* staticTree;
	Tree* dynamicTree;
	bool staticTreeDirty;
	bool dynamicTreeDirty;
};