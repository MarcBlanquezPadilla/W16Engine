#pragma once
#include <vector>
#include <string>
#include <map>

#include "imgui.h"

union SDL_Event;
class UIWindow;
class HierarchyWindow;
class SceneWindow;
class InspectorWindow;
class ProjectWindow;
class ToolbarWindow;

enum Menu
{
	File,
	View,
	Help
};

enum Theme
{
	Dark,
	Light,
	Cyberpunk,
	Dracula,
	Custom
};

class Interface {
public:

	Interface();

	~Interface();

	bool Awake();

	bool PreUpdate();
	bool Update();
	bool PostUpdate();

	bool CleanUp();

	void HandleInput(SDL_Event* event);
	void SetupDockspace(ImGuiID dockspace_id);
	void SetupImGuiStyle();
	void ApplyTheme(Theme theme);

	//GETTERS
	bool IsHierarchyFocused();
	bool IsSceneFocused();

private:
	void SetDarkTheme();
	void SetLightTheme();
	void SetCyberpunkTheme();
	void SetDraculaTheme();


private:
	
	Theme currentTheme;
	HierarchyWindow* hierarchy = nullptr;
	SceneWindow* scene = nullptr;
	InspectorWindow* inspector = nullptr;
	ProjectWindow* project = nullptr;
	ToolbarWindow* toolBar = nullptr;

	std::map<Menu, std::vector<UIWindow*>> windows;

    bool setDefaultUI = false;
	ImGuiIO* io = nullptr;

	bool showSaveSceneModal = false;
	char saveSceneNameBuffer[64] = "NewScene";
};