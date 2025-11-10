#pragma once
#include "Module.h"
#include <glm/gtc/matrix_transform.hpp>

class Camera : public Module
{
public:

	Camera(bool startEnabled);

	virtual ~Camera();

	bool Awake();

	bool PreUpdate();

	bool CleanUp();

	glm::mat4 GetViewMatrix() const { return viewMatrix; }
	glm::mat4 GetProjectionMatrix() const { return projectionMatrix; }

	void LockCamera(bool _lockCamera) { lockCamera = _lockCamera; }

private:
	void CalcMouseVectors();

public: 
	bool windowChanged;
	bool viewChanged;

private:
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;

	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 up;
	glm::vec3 right;

	float speed;
	float speedMultiplier;

	float mouseSensibility;
	float yaw;
	float pitch;

	float fieldOfView;
	float maxFieldOfView;
	float minFieldOfView;
	float zoomSpeed;

	float focusDistance;
	float orbitDistance;

	bool orbit;
	bool move;
	bool zoom;
	bool focus;

	bool shouldBeRelative;
	bool mouseCaptured;
	bool lockCamera;
};