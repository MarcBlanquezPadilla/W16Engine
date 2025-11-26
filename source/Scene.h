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
	void CollectGameObjectsRecursive(GameObject* go, std::vector<GameObject*>& list);
	void AddGameObject(GameObject* gameObject);

	//TREE
	void RebuildTree();
	void MarkStaticTreeDirty() { staticTreeDirty = true; }
	
	//RAY
	void QueryRay(Ray ray, std::vector<GameObject*>& results);
	void QueryRayToStatic(Ray ray, std::vector<GameObject*>& results);
	void QueryRayToDynamic(Ray ray, std::vector<GameObject*>& results);

	//GETTERS
	std::vector<GameObject*> GetGameObjects() { return gameObjects; }
	AABB GetWorldLimits();
	std::vector<GameObject*> GetAllGameObjects();
	std::vector<GameObject*> GetStaticGameObjects();
	std::vector<GameObject*> GetDynamicGameObjects();

	//EVENTS
	void OnEvent(const Event& event) override;

private:
	std::vector<GameObject*> gameObjects;

	Tree* staticTree;
	bool staticTreeDirty;
};