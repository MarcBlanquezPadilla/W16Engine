#include "Editor.h"
#include "Engine.h"
#include "Input.h"
#include "Log.h" 
#include "Render.h" 
#include "Window.h"

#include "Camera.h"
#include "Scene.h"
#include "GameObject.h"
#include "components/Transform.h"

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
#include "ImGuizmo.h"

#include "glm/gtc/type_ptr.hpp"
#include <cmath>





Editor::Editor(bool startEnabled) : Module(startEnabled)
{
	name = "Editor";
}

Editor::~Editor() {}

bool Editor::Awake()
{
	LOG("Initializing ImGui");
	bool ret = true;

	//IMGUI SETUP
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io_ref = ImGui::GetIO();
	this->io = &io_ref;
	io->IniFilename = NULL;

	io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();

	SDL_Window* window = Engine::GetInstance().window->window;
	SDL_GLContext gl_context = Engine::GetInstance().window->context;
	const char* glsl_version = "#version 460";

	if (!ImGui_ImplSDL3_InitForOpenGL(window, gl_context))
	{
		LOG("Error initializing ImGui_ImplSDL3_InitForOpenGL");
		ret = false;
	}

	if (!ImGui_ImplOpenGL3_Init(glsl_version))
	{
		LOG("Error initializing ImGui_ImplOpenGL3_Init");
		ret = false;
	}

	//CREATE WINDOWS
	windows[Menu::View].push_back(new ConfigWindow(false));
	windows[Menu::View].push_back(new ConsoleWindow(true));
	windows[Menu::View].push_back(new HierarchyWindow(true));
	windows[Menu::View].push_back(new InspectorWindow(true));

	windows[Menu::Help].push_back(new AboutWindow(false));

	setDefaultUI = true;

	return ret;
}

bool Editor::PreUpdate()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	return true;
}

bool Editor::Update(float dt)
{
	ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(0U, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

	if (setDefaultUI)
	{
		SetupDockspace(dockspace_id);
		setDefaultUI = false;
	}

	GameObject* selectedGameObject = Engine::GetInstance().scene->GetSelectedGameObject();
	Camera* camera = Engine::GetInstance().camera;

	if (selectedGameObject != nullptr)
	{
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_DOWN) currentGizmoOperation = ImGuizmo::TRANSLATE;
		else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_E) == KEY_DOWN) currentGizmoOperation = ImGuizmo::SCALE;
		else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_R) == KEY_DOWN) currentGizmoOperation = ImGuizmo::ROTATE;

		Transform* transform = (Transform*)selectedGameObject->GetComponent(ComponentType::Transform);
		if (transform)
		{
			ImGuizmo::SetOrthographic(false);

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGuizmo::SetRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);

			const glm::mat4& viewMatrix = camera->GetViewMatrix();
			const glm::mat4& projectionMatrix = camera->GetProjectionMatrix();
			glm::mat4 modelMatrix = transform->GetLocalMatrix();

			ImGuizmo::Manipulate(
				glm::value_ptr(viewMatrix),
				glm::value_ptr(projectionMatrix),
				currentGizmoOperation,
				ImGuizmo::LOCAL,
				glm::value_ptr(modelMatrix)
			);

			bool isGuizmoUsing = ImGuizmo::IsUsing();
			camera->LockCamera(isGuizmoUsing);
			if (isGuizmoUsing)
			{
				glm::vec3 newPos, newEulerRot, newScale;

				ImGuizmo::DecomposeMatrixToComponents(
					glm::value_ptr(modelMatrix),
					glm::value_ptr(newPos),
					glm::value_ptr(newEulerRot),
					glm::value_ptr(newScale)
				);

				transform->SetPosition(newPos);
				transform->SetEulerRotation(newEulerRot);
				transform->SetScale(newScale);
			}
		}
	}

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
				SDL_OpenURL("https://github.com/MarcBlanquezPadilla/W16Engine/blob/main/README.md");
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

		if (ImGui::BeginMenu("Preferences"))
		{
			if (ImGui::MenuItem("Set Default Interface"))
			{
				setDefaultUI = true;
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
	LOG("Turning off ImGui");
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	return true;
}

void Editor::HandleInput(SDL_Event* event)
{
	ImGui_ImplSDL3_ProcessEvent(event);
}

void Editor::SetupDockspace(unsigned int dockspace_id)
{
	for (auto& window : windows[Menu::Help])
	{
		window->is_active = window->defaultEnabled;
	}

	for (auto& window : windows[Menu::View])
	{
		window->is_active = window->defaultEnabled;
	}

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::DockBuilderRemoveNode(dockspace_id);
	ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);
	ImGuiID dock_main_id = dockspace_id;
	ImGuiID dock_left_id;
	ImGuiID dock_right_id;
	ImGuiID dock_bottom_id;

	dock_left_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.1666f, nullptr, &dock_main_id);

	dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.20f, nullptr, &dock_main_id);

	dock_bottom_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.30f, nullptr, &dock_main_id);

	ImGui::DockBuilderDockWindow("Hierarchy", dock_left_id);
	ImGui::DockBuilderDockWindow("Inspector", dock_right_id);
	ImGui::DockBuilderDockWindow("Console", dock_bottom_id);
	ImGui::DockBuilderFinish(dockspace_id);
}