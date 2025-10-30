#include "Editor.h"
#include "Engine.h"
#include "Log.h" 
#include "Render.h" 
#include "Window.h"

#include "windows/UIWindow.h"
#include "windows/ConfigWindow.h"
#include "windows/ConsoleWindow.h"
#include "windows/AboutWindow.h"
#include "windows/HierarchyWindow.h"
#include "windows/InspectorWindow.h"

#include <SDL3/SDL_events.h>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"


Editor::Editor(bool startEnabled) : Module(startEnabled)
{
	name = "Editor";
}

Editor::~Editor() {}

bool Editor::Awake()
{
	LOG("Initializing ImGui");
	bool ret = true;

	//SETUP DE IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io_ref = ImGui::GetIO();
	this->io = &io_ref;

	io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();

	SDL_Window* window = Engine::GetInstance().window->window;
	SDL_GLContext gl_context = Engine::GetInstance().window->context;
	const char* glsl_version = "#version 460";

	if (!ImGui_ImplSDL3_InitForOpenGL(window, gl_context))
	{
		LOG("Error al inicializar ImGui_ImplSDL3_InitForOpenGL");
		ret = false;
	}

	if (!ImGui_ImplOpenGL3_Init(glsl_version))
	{
		LOG("Error al inicializar ImGui_ImplOpenGL3_Init");
		ret = false;
	}

	//CREAR VENTANAS
	//VIEW
	windows[Menu::View].push_back(new ConfigWindow());
	windows[Menu::View].push_back(new ConsoleWindow());
	windows[Menu::View].push_back(new HierarchyWindow());
	windows[Menu::View].push_back(new InspectorWindow());
	
	//HELP
	windows[Menu::Help].push_back(new AboutWindow());

	return ret;
}

bool Editor::PreUpdate()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	return true;
}

bool Editor::Update(float dt)
{
	ImGui::DockSpaceOverViewport(0U,nullptr,ImGuiDockNodeFlags_PassthruCentralNode);

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				Engine::GetInstance().QuitApplication();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			for (auto& window : windows[Menu::View])
			{
				ImGui::MenuItem(window->name, NULL, &window->is_active);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("Documentation"))
			{
				SDL_OpenURL("https://github.com/MarcBlanquezPadilla/W16Engine");
			}
			if (ImGui::MenuItem("Report a bug"))
			{
				SDL_OpenURL("https://github.com/MarcBlanquezPadilla/W16Engine/issues");
			}
			if (ImGui::MenuItem("Download latest"))
			{
				SDL_OpenURL("https://github.com/MarcBlanquezPadilla/W16Engine/releases");
			}
			for (auto& window : windows[Menu::Help])
			{
				ImGui::MenuItem(window->name, NULL, &window->is_active);
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	

	for (auto const& pair : windows)
	{
		const std::vector<UIWindow*>& windows = pair.second;

		for (auto& window : windows)
		{
			if (window->is_active)
			{
				window->Draw();
			}
		}
	}

	return true;
}

bool Editor::PostUpdate()
{

	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	return true;
}

bool Editor::CleanUp()
{
	LOG("Apagando ImGui");
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	return true;
}

void Editor::HandleInput(SDL_Event* event)
{
	// Pasa el evento de SDL a ImGui
	ImGui_ImplSDL3_ProcessEvent(event);
}