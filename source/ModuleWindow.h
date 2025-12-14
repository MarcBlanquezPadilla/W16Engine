#pragma once
#include "Module.h"
#include "Global.h"
#include "EventListener.h"
#include "SDL3/SDL.h"
#include <string>

class ModuleWindow : public Module, public EventListener
{
public:

	ModuleWindow(bool startEnabled);

	~ModuleWindow() override;

	bool Awake() override;

	bool CleanUp() override;

	void SetTitle(const char* title);

	void GetWindowSize(int& width, int& height) const;

	int GetScale() const;

	std::string GetSDLVersion() { return sdlVersion; }

	void Swap() { SDL_GL_SwapWindow(window); }

	std::string GetCPU();
	std::string GetRAM();

	//EVENTS
	void OnEvent(const Event& event) override;


public:

	SDL_Window* window;
	SDL_GLContext context;

	std::string title;
	int width = WINDOW_WIDTH;
	int height = WINDOW_HEIGHT;
	int scale = WINDOW_SCALE;

	std::string sdlVersion;
	std::string cpu_brand;
	std::string ram;
};