#pragma once
#include "Module.h"
#include "EventListener.h"
#include "utils/Config.h"
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
	bool Update();
	bool PostUpdate();

	bool NewScene();

	bool CleanUp();

	//GAMEOBJECT
	void AddGameObject(GameObject* gameObject);
	void DestroyGameObject(GameObject* gameObject);
	void RemoveGameObject(GameObject* gameObject);
	void AddToRoots(GameObject* go);
	void RemoveFromRoots(GameObject* go);

	//TREE
	void RebuildTree();
	void MarkStaticTreeDirty() { staticTreeDirty = true; }
	
	//RAY
	void QueryRay(Ray ray, std::vector<GameObject*>& results);
	void QueryRayToStatic(Ray ray, std::vector<GameObject*>& results);
	void QueryRayToDynamic(Ray ray, std::vector<GameObject*>& results);

	//GETTERS
	std::vector<GameObject*> GetRootGameObjects() const { return rootGameObjects; };
	std::vector<GameObject*> GetAllGameObjects() const { return allGameObjects; };
	std::vector<GameObject*> GetStaticGameObjects() const { return staticGameObjects; };
	std::vector<GameObject*> GetDynamicGameObjects() const { return dynamicGameObjects; };
	AABB GetWorldLimits();
	Tree* GetTree() { return staticTree; }

	//EVENTS
	void OnEvent(const Event& event) override;

public: 
	Config sceneBackup;

private:
	std::vector<GameObject*> allGameObjects;
	std::vector<GameObject*> rootGameObjects;
	std::vector<GameObject*> dynamicGameObjects;
	std::vector<GameObject*> staticGameObjects;
	std::vector<GameObject*> objectsPendingToDelete;

	Tree* staticTree;
	bool staticTreeDirty;

	
};