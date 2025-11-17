#pragma once
#include "Module.h"
#include <vector>
#include <map>

#include "glm/glm.hpp"
#include "imgui.h"
#include "ImGuizmo.h"

union SDL_Event;
class UIWindow;

enum Menu
{
	File,
	View,
	Help
};

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

	void HandleInput(SDL_Event* event);
	void SetupDockspace(ImGuiID dockspace_id);

	void TestMouseRay(int mouseX, int mouseY);

	//CAMBIAR A WINDOW SCENE
	ImGuizmo::OPERATION currentGizmoOperation = ImGuizmo::TRANSLATE;

private:
	ImGuiIO* io = nullptr;
	std::map<Menu, std::vector<UIWindow*>> windows;

	glm::vec3 startLastRay;
	glm::vec3 endLastRay;

	bool debugRay;
	bool debugAABB;
    bool setDefaultUI = false;
};