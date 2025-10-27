#include "Window.h"
#include "Engine.h"
#include "Log.h"
#include "Global.h"
#include "Input.h"

Window::Window(bool startEnabled) : Module(startEnabled)
{
	window = NULL;
	name = "window";
}

Window::~Window()
{

}

bool Window::Awake()
{

	bool ret = true;

	width = WINDOW_WIDTH;
	height = WINDOW_HEIGHT;

	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		LOG("SDL_VIDEO could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		SDL_WindowFlags flags = SDL_WINDOW_OPENGL;

		flags |= SDL_WINDOW_RESIZABLE;

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

		window = SDL_CreateWindow("Platform Game", width, height, flags);

		context = SDL_GL_CreateContext(window);

		SDL_SetWindowRelativeMouseMode(window, true);

		if (window == NULL)
		{
			LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			ret = false;
		}
	}

	return ret;
}

bool Window::PostUpdate()
{
	SDL_GL_SwapWindow(window);
	return true;
}

bool Window::CleanUp()
{
	LOG("Destroying SDL window and quitting all SDL systems");

	if (context != NULL)
	{
		SDL_GL_DestroyContext(context);
	}

	if (window != NULL)
	{
		SDL_DestroyWindow(window);
	}

	SDL_Quit();
	return true;
}

void Window::SetTitle(const char* new_title)
{
	SDL_SetWindowTitle(window, new_title);
}

void Window::GetWindowSize(int& width, int& height) const
{
	width = this->width;
	height = this->height;
}

int Window::GetScale() const
{
	return scale;
}