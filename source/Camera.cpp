#include "Camera.h"
#include <SDL3/sdl.h>
#include "Log.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Global.h"
#include "Input.h"
#include "Engine.h"
#include "Render.h"
#include "Scene.h"
#include "Window.h"
#include "GameObject.h"
#include "components/Component.h"
#include "components/Transform.h"


Camera::Camera(bool startEnabled) : Module(startEnabled)
{
	name = "Camera";
}

Camera::~Camera()
{

}

bool Camera::Awake()
{
	bool ret = true;
	
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

	forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	forward.y = sin(glm::radians(pitch));
	forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	
	right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
	up = glm::normalize(glm::cross(right, forward));

	viewMatrix = glm::lookAt(
		position,
		position + forward,
		up
	);

	projectionMatrix = glm::perspective(
		glm::radians(fieldOfView),
		(float)Engine::GetInstance().window->width / (float)Engine::GetInstance().window->height,
		0.1f,
		1000.0f
	);

	Engine::GetInstance().render->UpdateViewMatix(viewMatrix);
	Engine::GetInstance().render->UpdateProjectionMatix(projectionMatrix);

	return ret;
}


bool Camera::PreUpdate()
{
	bool ret = true;

	if (lockCamera)
	{
		return ret;
	}

	SDL_SetWindowRelativeMouseMode(Engine::GetInstance().window->window, false);

	GameObject* gameObject = Engine::GetInstance().scene->GetSelectedGameObject();

	bool shouldBeRelative = (
		(Engine::GetInstance().input->GetMouseButtonDown(3) == KEY_REPEAT) ||
		(Engine::GetInstance().input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT && Engine::GetInstance().input->GetMouseButtonDown(1) == KEY_REPEAT && gameObject));

	if (shouldBeRelative && !mouseCaptured)
	{
		SDL_SetWindowRelativeMouseMode(Engine::GetInstance().window->window, true);

		float tempX, tempY;
		SDL_GetRelativeMouseState(&tempX, &tempY);
		mouseCaptured = true;
	}
	else if (mouseCaptured && !shouldBeRelative)
	{
		SDL_SetWindowRelativeMouseMode(Engine::GetInstance().window->window, false);
		mouseCaptured = false;
	}

	
	orbit = Engine::GetInstance().input->GetMouseButtonDown(1) == KEY_REPEAT && Engine::GetInstance().input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT && gameObject;
	move = Engine::GetInstance().input->GetMouseButtonDown(3) == KEY_REPEAT;
	focus = Engine::GetInstance().input->GetKey(SDL_SCANCODE_F) == KEY_DOWN && gameObject;
	zoom = Engine::GetInstance().input->GetMouseWheelY() != 0;

	//FOCUS
	if (focus)
	{
		Transform* transform = (Transform*)gameObject->GetComponent(ComponentType::Transform);
		if (transform)
		{
			glm::vec3 targetPosition = transform->GetPosition();

			forward = glm::normalize(targetPosition - position);

			position = targetPosition - (forward * focusDistance);

			yaw = glm::degrees(atan2(forward.z, forward.x));
			pitch = glm::degrees(asin(forward.y));

			right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
			up = glm::normalize(glm::cross(right, forward));

			viewChanged = true;
		}
		focus = false;
	}
	//ORBIT
	else if(orbit)
	{	
		CalcMouseVectors();

		Transform* transform = (Transform*)gameObject->GetComponent(ComponentType::Transform);
		if (transform)
		{
			glm::vec3 vectorOrbit = { transform->GetPosition().x - position.x, transform->GetPosition().y - position.y, transform->GetPosition().z - position.z };
			orbitDistance = glm::length(vectorOrbit);
			glm::vec3 targetPosition = transform->GetPosition();
			position = targetPosition - (forward * orbitDistance);
			viewChanged = true;
		}
	}
	//WASD AND MOUSE MOVEMENT
	else if (move)
	{
		
		CalcMouseVectors();
		
		bool wPressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT;
		bool sPressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT;
		bool aPressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT;
		bool dPressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT;
		bool shift = (Engine::GetInstance().input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT ||
			Engine::GetInstance().input->GetKey(SDL_SCANCODE_RSHIFT) == KEY_REPEAT);

			
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
		float mouseWheel = Engine::GetInstance().input->GetMouseWheelY();
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
		viewMatrix = glm::lookAt(
			position,
			position + forward,
			up
		);
		Engine::GetInstance().render->UpdateViewMatix(viewMatrix);
		viewChanged = false;
	}
	
	if (windowChanged)
	{
		projectionMatrix = glm::perspective(
			glm::radians(fieldOfView),
			(float)Engine::GetInstance().window->width / (float)Engine::GetInstance().window->height,
			0.1f,
			1000.0f
		);
		Engine::GetInstance().render->UpdateProjectionMatix(projectionMatrix);
		windowChanged = false;
	}

	return ret;
}

void Camera::CalcMouseVectors()
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


bool Camera::CleanUp()
{
	bool ret = true;

	

	return ret;
}
