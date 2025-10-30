#pragma once
#include "Module.h"
#include <vector>
#include <map>

struct ImGuiIO;
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

private:
	ImGuiIO* io = nullptr;
	std::map<Menu, std::vector<UIWindow*>> windows;
};