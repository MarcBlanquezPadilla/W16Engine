#include "ModuleWindow.h"
#include "Engine.h"
#include "ModuleEvents.h"
#include "utils/Log.h"
#include "Global.h"
#include "ModuleInput.h"
#include "SDL3/SDL.h"
#include <windows.h>

ModuleWindow::ModuleWindow(bool startEnabled) : Module(startEnabled)
{
	window = NULL;
	name = "window";
}

ModuleWindow::~ModuleWindow()
{
	CleanUp();
}

bool ModuleWindow::Awake()
{
	bool ret = true;

	width = WINDOW_WIDTH;
	height = WINDOW_HEIGHT;

	//INIT SDL
	LOG("Initializing SDL3");
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

		window = SDL_CreateWindow("W16Engine", width, height, flags);

		context = SDL_GL_CreateContext(window);

		SDL_SetWindowRelativeMouseMode(window, false);

		if (window == NULL)
		{
			LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			ret = false;
		}

		const int linked_version_int = SDL_GetVersion();

		int major = SDL_VERSIONNUM_MAJOR(linked_version_int);
		int minor = SDL_VERSIONNUM_MINOR(linked_version_int);
		int patch = SDL_VERSIONNUM_MICRO(linked_version_int);

		sdlVersion = std::to_string(major) + "." +
			std::to_string(minor) + "." +
			std::to_string(patch);
	}

	//RAM
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);

	float totalRAM_gb = (float)memInfo.ullTotalPhys / (1024 * 1024 * 1024);

	char ram_buffer[32];
	sprintf_s(ram_buffer, "%.2f GB", totalRAM_gb);
	ram = ram_buffer;

	//CPU
	HKEY hKey;
	DWORD bufferSize = 1024;
	char cpuName[1024] = { 0 };
	const char* keyPath = "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0";

	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		if (RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)cpuName, &bufferSize) == ERROR_SUCCESS)
		{
			cpu_brand = cpuName;
		}
		else
		{
			cpu_brand = "Error al leer el nombre de la CPU";
		}
		RegCloseKey(hKey);
	}
	else
	{
		cpu_brand = "Error al abrir el registro";
	}

	//EVENTS
	Engine::GetInstance().moduleEvents->Subscribe(Event::Type::WindowResize, this);

	return ret;
}

bool ModuleWindow::CleanUp()
{
	Engine::GetInstance().moduleEvents->UnsubscribeAll(this);
	LOG("Destroying SDL window and quitting all SDL systems");

	if (context != NULL)
	{
		SDL_GL_DestroyContext(context);
	}

	if (window != NULL)
	{
		SDL_DestroyWindow(window);
	}

	context = nullptr;
	window = nullptr;

	SDL_Quit();
	return true;
}

void ModuleWindow::SetTitle(const char* new_title)
{
	SDL_SetWindowTitle(window, new_title);
}

void ModuleWindow::GetWindowSize(int& width, int& height) const
{
	width = this->width;
	height = this->height;
}

int ModuleWindow::GetScale() const
{
	return scale;
}

std::string ModuleWindow::GetRAM()
{
	return ram;
}

std::string ModuleWindow::GetCPU()
{
	return cpu_brand;
}

void ModuleWindow::OnEvent(const Event& event)
{
	switch (event.type)
	{
	case Event::Type::WindowResize:
	{
		{
			width = event.data.point.x;
			height = event.data.point.y;
		}
		break;
	}

	default:
		break;
	}
}