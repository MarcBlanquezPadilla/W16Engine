#include "Module.h"

struct SDL_Window;

class Window : public Module
{
public:

	Window(bool startEnabled);

	// Destructor
	virtual ~Window();

	// Called before render is available
	bool Awake();

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

	std::string title;
	int width = 1280;
	int height = 720;

	int scale = 1;
};