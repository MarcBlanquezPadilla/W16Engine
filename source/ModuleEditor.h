#pragma once
#include "Module.h"
#include "EventListener.h"
#include <vector>
#include <map>

#include "glm/glm.hpp"
#include "imgui.h"
#include "ImGuizmo.h"

union SDL_Event;
class Interface;
class GameObject;
class EditorCamera;
class CameraLens;

class ModuleEditor : public Module, public EventListener
{
public:

	ModuleEditor(bool startEnabled);

	~ModuleEditor() override;

	bool Awake();

	bool PreUpdate();
	bool Update();
	bool PostUpdate();

	bool CleanUp();

	void TestMouseRayPicking(int mouseX, int mouseY, int width, int height);
	void TestMousePixelPicking(int mouseX, int mouseY);

	void HandleInput(SDL_Event* event);

	void SetSelected(GameObject* gameObject, bool eraseSelecteds = true);
	
	std::vector<GameObject*> GetSelectedGameObjects() { return selectedGameObjects; };

	EditorCamera* GetEditorCamera() { return editorCamera; };
	Interface* GetInterface() { return userInterface; };
	CameraLens* GetEditorCameraLens();

	//EVENTS
	void OnEvent(const Event& event) override;



public:

	ImGuizmo::OPERATION currentGizmoOperation = ImGuizmo::TRANSLATE;

	bool debugRay;
	bool debugTree;
	bool debugAABB;
	bool debugMesh;
	bool debugNormal;
	bool debugGrid;
	bool debugCamera;
	bool debugChecker;

	int gridSize;
	int gridRows;
	int gridColumns;

private:
	
	Interface* userInterface;
	EditorCamera* editorCamera;

	glm::vec3 startLastRay;
	glm::vec3 endLastRay;

    bool setDefaultUI = false;

	std::vector<GameObject*> selectedGameObjects;
};