#pragma once
#include "Module.h"
#include "SDL3/SDL.h"
#include <string>

class Window : public Module
{
public:

	Window(bool startEnabled);

	virtual ~Window();

	bool Awake();

	bool PostUpdate();

	bool CleanUp();

	void SetTitle(const char* title);

	void GetWindowSize(int& width, int& height) const;

	int GetScale() const;

	std::string GetSDLVersion() { return sdlVersion; }

	std::string GetCPU();
	std::string GetRAM();


public:

	SDL_Window* window;
	SDL_GLContext context;

	std::string title;
	int width = 1280;
	int height = 720;

	int scale = 1;

	std::string sdlVersion;
	std::string cpu_brand;
	std::string ram;
};