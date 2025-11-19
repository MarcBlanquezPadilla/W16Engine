#pragma once
#include "Module.h"
#include "Global.h"
#include "EventListener.h"
#include "SDL3/SDL.h"
#include <string>

class Window : public Module, public EventListener
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