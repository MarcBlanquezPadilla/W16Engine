#pragma once
#include "Module.h"
#include <vector>
#include <map>

#include "glm/glm.hpp"
#include "imgui.h"
#include "ImGuizmo.h"

union SDL_Event;
class Interface;

class Editor : public Module
{
public:

	Editor(bool startEnabled);

	virtual ~Editor();

	bool Awake();

	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();

	bool CleanUp();

	void TestMouseRay(int mouseX, int mouseY);

	void HandleInput(SDL_Event* event);

	//CAMBIAR A WINDOW SCENE
	ImGuizmo::OPERATION currentGizmoOperation = ImGuizmo::TRANSLATE;

private:

	Interface* userInterface;

	glm::vec3 startLastRay;
	glm::vec3 endLastRay;

	bool debugRay;
	bool debugAABB;
	bool debugMesh;
    bool setDefaultUI = false;
};