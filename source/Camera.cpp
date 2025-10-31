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
		(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
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

	if (Engine::GetInstance().input->GetMouseButtonDown(3) == KEY_DOWN)
	{
		SDL_SetWindowRelativeMouseMode(Engine::GetInstance().window->window, true);

		float tempX, tempY;
		SDL_GetRelativeMouseState(&tempX, &tempY);
	}

	if (Engine::GetInstance().input->GetMouseButtonDown(3) == KEY_UP)
	{
		SDL_SetWindowRelativeMouseMode(Engine::GetInstance().window->window, false);
	}

	//FOCUS
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F) == KEY_DOWN)
	{
		GameObject* gameObject = Engine::GetInstance().scene->GetSelectedGameObject();
		if (gameObject)
		{
			Transform* transform = (Transform*)gameObject->GetComponent(ComponentType::Transform);
			if (transform)
			{
				glm::vec3 targetPosition = transform->position;

				forward = glm::normalize(targetPosition - position);

				position = targetPosition - (forward * focusDistance);

				yaw = glm::degrees(atan2(forward.z, forward.x));
				pitch = glm::degrees(asin(forward.y));

				right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
				up = glm::normalize(glm::cross(right, forward));

				viewChanged = true;
			}
		}
	}
	if (Engine::GetInstance().input->GetMouseButtonDown(3) == KEY_REPEAT)
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


		//KEYS PRESSEDS
		bool wPressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT;
		bool sPressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT;
		bool aPressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT;
		bool dPressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT;
		bool shift = (Engine::GetInstance().input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT ||
			Engine::GetInstance().input->GetKey(SDL_SCANCODE_RSHIFT) == KEY_REPEAT);

		//WASD MOVEMENT
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

	if (viewChanged)
	{
		//UPDATE VIEW MATRIX
		viewMatrix = glm::lookAt(
			position,
			position + forward,
			up
		);
		Engine::GetInstance().render->UpdateViewMatix(viewMatrix);
		viewChanged = false;
	}

	
	float mouseWheel = Engine::GetInstance().input->GetMouseWheelY();

	if (mouseWheel != 0) windowChanged = true;
	
	if (windowChanged)
	{
		//ZOOM
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

		projectionMatrix = glm::perspective(
			glm::radians(fieldOfView),
			(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
			0.1f,
			1000.0f
		);
		Engine::GetInstance().render->UpdateProjectionMatix(projectionMatrix);
	}

	return ret;
}

bool Camera::CleanUp()
{
	bool ret = true;

	

	return ret;
}
