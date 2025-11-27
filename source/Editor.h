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

class Editor : public Module, public EventListener
{
public:

	Editor(bool startEnabled);

	virtual ~Editor();

	bool Awake();

	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();

	bool CleanUp();

	void TestMouseRay(int mouseX, int mouseY, int width, int height);

	void HandleInput(SDL_Event* event);

	void SetSelected(GameObject* gameObject);
	
	GameObject* GetSelectedGameObject() { return selectedGameObject; };

	EditorCamera* GetEditorCamera();
	CameraLens* GetEditorCameraLens();

	//EVENTS
	void OnEvent(const Event& event) override;



public:

	ImGuizmo::OPERATION currentGizmoOperation = ImGuizmo::TRANSLATE;

	bool debugRay;
	bool debugAABB;
	bool debugMesh;

private:
	Interface* userInterface;
	EditorCamera* editorCamera;

	glm::vec3 startLastRay;
	glm::vec3 endLastRay;

    bool setDefaultUI = false;

	GameObject* selectedGameObject;
};