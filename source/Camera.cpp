#include "Camera.h"
#include <SDL3/sdl.h>
#include "Log.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Global.h"
#include "Input.h"
#include "Engine.h"
#include "Render.h"


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
	
	projectionMatrix = glm::perspective(
		glm::radians(45.0f), 
		(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
		0.1f,
		1000.0f
	);

	return ret;
}


bool Camera::PreUpdate()
{
	bool ret = true;

	//IF MOVED

	position = glm::vec3(0.0f, 0.0f, 10.0f);
	forward = glm::vec3(0.0f, 0.0f, -1.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	right = glm::cross(forward, up);


	glm::mat4 viewMatrix = glm::lookAt(
		position,
		position+forward,
		up
	);

	

	//IF WINDOW CHANGED
	projectionMatrix = glm::perspective(
		glm::radians(45.0f),
		(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
		0.1f,
		1000.0f
	);

	Engine::GetInstance().render->UpdateCameraMatix(projectionMatrix, viewMatrix);
		//POS
		//FORWARD
		//UP
		//RIGHT

	return ret;
}

bool Camera::CleanUp()
{
	bool ret = true;

	

	return ret;
}
