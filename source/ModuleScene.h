#pragma once
#include "Module.h"
#include "EventListener.h"
#include <vector>

class GameObject;
class Tree;
class AABB;
struct Ray;

class ModuleScene : public Module, public EventListener
{
public:

	ModuleScene(bool startEnabled) ;

	virtual ~ModuleScene();

	bool Awake();
	bool Start();

	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();

	bool NewScene();

	bool CleanUp();

	//GAMEOBJECT
	void CollectGameObjectsRecursive(GameObject* go, std::vector<GameObject*>& list);
	void AddGameObject(GameObject* gameObject);
	void DestroyGameObject(GameObject* gameObject);
	void RemoveGameObject(GameObject* gameObject);

	//TREE
	void RebuildTree();
	void MarkStaticTreeDirty() { staticTreeDirty = true; }
	
	//RAY
	void QueryRay(Ray ray, std::vector<GameObject*>& results);
	void QueryRayToStatic(Ray ray, std::vector<GameObject*>& results);
	void QueryRayToDynamic(Ray ray, std::vector<GameObject*>& results);

	//GETTERS
	std::vector<GameObject*> GetGameObjects() { return gameObjects; }
	std::vector<GameObject*> GetAllGameObjects();
	std::vector<GameObject*> GetStaticGameObjects() const { return staticGameObjects; };
	std::vector<GameObject*> GetDynamicGameObjects() const { return dynamicGameObjects; };
	AABB GetWorldLimits();
	Tree* GetTree() { return staticTree; }

	//EVENTS
	void OnEvent(const Event& event) override;

private:
	std::vector<GameObject*> gameObjects;
	std::vector<GameObject*> dynamicGameObjects;
	std::vector<GameObject*> staticGameObjects;
	std::vector<GameObject*> objectsPendingToDelete;

	Tree* staticTree;
	bool staticTreeDirty;
};