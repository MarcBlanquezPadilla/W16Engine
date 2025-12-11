#pragma once
#include <vector>
#include <map>

#include "imgui.h"

union SDL_Event;
class UIWindow;

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

private:
	void SetDarkTheme();
	void SetLightTheme();
	void SetCyberpunkTheme();
	void SetDraculaTheme();

private:
	
	Theme currentTheme;
	
	std::map<Menu, std::vector<UIWindow*>> windows;
	UIWindow* toolBar;

    bool setDefaultUI = false;
	ImGuiIO* io = nullptr;
};