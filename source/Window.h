#pragma once
#include "Module.h"
#include "SDL3/SDL.h"

struct SDL_Window;

class Window : public Module
{
public:

	Window(bool startEnabled);

	// Destructor
	virtual ~Window();

	// Called before render is available
	bool Awake();

	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// Changae title
	void SetTitle(const char* title);

	// Retrive window size
	void GetWindowSize(int& width, int& height) const;

	// Retrieve window scale
	int GetScale() const;

public:

	SDL_Window* window;
	SDL_GLContext context;

	std::string title;
	int width = 1280;
	int height = 720;

	int scale = 1;
};