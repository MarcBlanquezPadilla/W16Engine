#include "EditorCamera.h"
#include <SDL3/sdl.h>
#include "utils/Log.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Global.h"
#include "ModuleInput.h"
#include "Engine.h"
#include "CameraLens.h"
#include "ModuleEditor.h"
#include "ModuleRender.h"
#include "ModuleScene.h"
#include "ModuleWindow.h"
#include "GameObject.h"
#include "components/Component.h"
#include "components/Transform.h"
#include "utils/Frustum.h"
#include "utils/Ray.h"
#include "ModuleEvents.h"

EditorCamera::EditorCamera()
{
	
}

EditorCamera::~EditorCamera()
{

}

bool EditorCamera::Awake()
{
	bool ret = true;
	
	cameraLens = new CameraLens();
	cameraLens->SetDebugCamera(true);
	Engine::GetInstance().moduleRender->AddCamera(cameraLens);

	int w, h;
	Engine::GetInstance().moduleWindow->GetWindowSize(w, h);
	cameraLens->SetRenderTarget(w, h);
	cameraLens->depth = -1;

	position = glm::vec3(0.0f, 0.0f, 10.0f);
	forward = glm::vec3(0.0f, 0.0f, -1.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	right = glm::cross(forward, up);

	speed = 1;
	speedMultiplier = 5;
	yaw = -90.0f;
	pitch = 0.0f;
	mouseSensibility = 0.1f;
	fieldOfView = 45.0f;
	maxFieldOfView = 45.0f;
	minFieldOfView = 10.0f;
	fieldOfView = 45.0f;
	zoomSpeed = 3.0f;
	focusDistance = 5.0f;

	focus = false;
	move = false;
	zoom = false;
	orbit = false;

	shouldBeRelative = false;
	mouseCaptured = false;
	lockCamera = false;

	//EVENTS
	Engine::GetInstance().moduleEvents->Subscribe(Event::Type::WindowResize, this);

	return ret;
}


bool EditorCamera::PreUpdate()
{
	bool ret = true;

	if (lockCamera)
	{
		return ret;
	}

	

	std::vector<GameObject*> gameObjects = Engine::GetInstance().moduleEditor->GetSelectedGameObjects();

	bool shouldBeRelative = (
		(Engine::GetInstance().moduleInput->GetMouseButtonDown(3) == KEY_REPEAT) ||
		(Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT && Engine::GetInstance().moduleInput->GetMouseButtonDown(1) == KEY_REPEAT && !gameObjects.empty()));

	if (shouldBeRelative && !mouseCaptured)
	{

		float tempX, tempY;
		SDL_GetRelativeMouseState(&tempX, &tempY);
		SDL_SetWindowRelativeMouseMode(Engine::GetInstance().moduleWindow->window, true);
		mouseCaptured = true;
	}
	else if (mouseCaptured && !shouldBeRelative)
	{
		SDL_SetWindowRelativeMouseMode(Engine::GetInstance().moduleWindow->window, false);
		mouseCaptured = false;
	}

	orbit = Engine::GetInstance().moduleInput->GetMouseButtonDown(1) == KEY_REPEAT && Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT && !gameObjects.empty();
	move = Engine::GetInstance().moduleInput->GetMouseButtonDown(3) == KEY_REPEAT;
	focus = Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_F) == KEY_DOWN && !gameObjects.empty();
	zoom = Engine::GetInstance().moduleInput->GetMouseWheelY() != 0;

	//FOCUS
	if (focus)
	{
		glm::vec3 centerPosition(0.0f);
		int validTransforms = 0;

		for (GameObject* go : gameObjects)
		{
			Transform* transform = (Transform*)go->GetComponent(ComponentType::Transform);
			if (transform)
			{
				centerPosition += transform->GetGlobalPosition();
				validTransforms++;
			}
		}

		if (validTransforms > 0)
		{
			centerPosition /= (float)validTransforms;

			forward = glm::normalize(centerPosition - position);
			position = centerPosition - (forward * focusDistance);

			yaw = glm::degrees(atan2(forward.z, forward.x));
			pitch = glm::degrees(asin(forward.y));

			right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
			up = glm::normalize(glm::cross(right, forward));

			viewChanged = true;
		}
		focus = false;
	}
	//ORBIT
	else if (orbit)
	{
		CalcMouseVectors();

		glm::vec3 centerPosition(0.0f);
		int validTransforms = 0;

		for (GameObject* go : gameObjects)
		{
			Transform* transform = (Transform*)go->GetComponent(ComponentType::Transform);
			if (transform)
			{
				centerPosition += transform->GetGlobalPosition();
				validTransforms++;
			}
		}

		if (validTransforms > 0)
		{
			centerPosition /= (float)validTransforms;

			glm::vec3 vectorOrbit = centerPosition - position;
			orbitDistance = glm::length(vectorOrbit);
			position = centerPosition - (forward * orbitDistance);
			viewChanged = true;
		}
	}
	//WASD AND MOUSE MOVEMENT
	else if (move)
	{
		CalcMouseVectors();

		bool wPressed = Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_W) == KEY_REPEAT;
		bool sPressed = Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_S) == KEY_REPEAT;
		bool aPressed = Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_A) == KEY_REPEAT;
		bool dPressed = Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_D) == KEY_REPEAT;
		bool shift = (Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT ||
			Engine::GetInstance().moduleInput->GetKey(SDL_SCANCODE_RSHIFT) == KEY_REPEAT);


		int xMovement = 0;
		if (aPressed && !dPressed) xMovement = -1;
		if (dPressed && !aPressed) xMovement = 1;

		int zMovement = 0;
		if (wPressed && !sPressed) zMovement = 1;
		if (sPressed && !wPressed) zMovement = -1;

		float dt = Engine::GetInstance().GetDtS();
		float finalSpeed = speed * dt * (shift ? speedMultiplier : 1.0f);

		if (wPressed)
			position += forward * finalSpeed;
		if (sPressed)
			position -= forward * finalSpeed;
		if (aPressed)
			position -= right * finalSpeed;
		if (dPressed)
			position += right * finalSpeed;
		viewChanged = true;
	}

	//ZOOM
	if (zoom)
	{
		float mouseWheel = Engine::GetInstance().moduleInput->GetMouseWheelY();
		if (mouseWheel < 0)
		{
			fieldOfView += zoomSpeed;
			if (fieldOfView > maxFieldOfView) fieldOfView = 45.0f;
		}
		else if (mouseWheel > 0)
		{
			fieldOfView -= zoomSpeed;
			if (fieldOfView < minFieldOfView) fieldOfView = 10.0f;
		}
		windowChanged = true;
	}

	//UPDATE MATRIX
	if (viewChanged)
	{
		cameraLens->LookAt(position, position + forward, up);
		viewChanged = false;
	}

	if (windowChanged)
	{
		cameraLens->SetPerspective(fieldOfView, (float)Engine::GetInstance().moduleWindow->width / (float)Engine::GetInstance().moduleWindow->height, 0.1f, 1000.0f);
		windowChanged = false;
	}

	return ret;
}

void EditorCamera::CalcMouseVectors()
{
	//MOUSE
	float mouseX, mouseY;
	SDL_GetRelativeMouseState(&mouseX, &mouseY);

	float xOffset = mouseX * mouseSensibility;
	float yOffset = mouseY * mouseSensibility;

	yaw += xOffset;
	pitch -= yOffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 newForward;
	newForward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	newForward.y = sin(glm::radians(pitch));
	newForward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	forward = glm::normalize(newForward);

	right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
	up = glm::normalize(glm::cross(right, forward));
}


bool EditorCamera::CleanUp()
{
	bool ret = true;

	Engine::GetInstance().moduleEvents->UnsubscribeAll(this);
	cameraLens->CleanUp();
	delete cameraLens;

	return ret;
}

void EditorCamera::OnEvent(const Event& event)
{
	switch (event.type)
	{
	case Event::Type::WindowResize:
	{
		cameraLens->SetRenderTarget(event.data.point.x, event.data.point.y);
		windowChanged = true;
		
		break;
	}

	default:
		break;
	}
}
