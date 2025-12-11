#include "../Engine.h"
#include "../ModuleTime.h"
#include "../ModuleScene.h"
#include "../ModuleLoader.h"

#include "ToolbarWindow.h"

#include "../utils/Log.h"
#include "../utils/Time.h"

#include "imgui.h"

ToolbarWindow::ToolbarWindow(bool active) : UIWindow("Toolbar", active)
{
	pauseOnPlay = false;
}

ToolbarWindow::~ToolbarWindow()
{

}

void ToolbarWindow::Draw()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImGuiWindowFlags toolbar_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar;

	ImGuiStyle& style = ImGui::GetStyle();
	float toolbarHeight = ImGui::GetFrameHeight() + (style.WindowPadding.y * 2.0f);


	ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + ImGui::GetFrameHeight()));
	ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, toolbarHeight));
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

	if (ImGui::Begin("Toolbar", nullptr, toolbar_flags))
	{
		//PROPERTIES
		float buttonWidth = 60.0f;
		float width = 200.0f;
		float off = (ImGui::GetContentRegionAvail().x - width) * 0.5f;
		if (off > 0.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

		//LOGIC
		bool isRunning = Engine::GetInstance().moduleTime->GetIsRunning();
		bool isPaused = Engine::GetInstance().moduleTime->GetIsPaused();
		
		//PLAY BUTTON
		if (isRunning) ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_ButtonActive]);
		const char* playButtonText = isRunning ? "Stop" : "Play";
		if (ImGui::Button(playButtonText, ImVec2(buttonWidth, 0)))
		{
			if (isRunning)
			{
				Engine::GetInstance().moduleScene->NewScene();
				Engine::GetInstance().moduleLoader->LoadSceneFromMemory(Engine::GetInstance().moduleScene->sceneBackup);
				Engine::GetInstance().moduleTime->Stop();
				pauseOnPlay = false;
			}
			else 
			{
				Engine::GetInstance().moduleScene->sceneBackup.Reset();
				Engine::GetInstance().moduleLoader->SaveSceneToMemory(Engine::GetInstance().moduleScene->sceneBackup);
				Engine::GetInstance().moduleTime->Play();
				if (pauseOnPlay) Engine::GetInstance().moduleTime->Pause();
			}
		}

		if (isRunning) ImGui::PopStyleColor();
		
		bool showPause = isRunning ? isPaused : pauseOnPlay;

		//PAUSE BUTTON
		ImGui::SameLine();
		if (showPause) ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_ButtonActive]);
		if (ImGui::Button("Pause", ImVec2(buttonWidth, 0)))
		{
			if (isRunning)
			{
				Engine::GetInstance().moduleTime->Pause();
			}
			else
			{
				pauseOnPlay = !pauseOnPlay;
			}
		}

		if (showPause) ImGui::PopStyleColor();
		
		

		//STEP BUTTON
		ImGui::SameLine();
		if (!isPaused) ImGui::BeginDisabled();

		if (ImGui::Button("Frame", ImVec2(buttonWidth, 0)))
		{
			Engine::GetInstance().moduleTime->Step();
		}

		if (!isPaused) ImGui::EndDisabled();

		ImGui::End();
		
	}
	ImGui::PopStyleVar();

	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);

	viewport->WorkPos.y += toolbarHeight;
	viewport->WorkSize.y -= toolbarHeight;
}