#pragma once
#include "Module.h"
#include "EventListener.h"
#include <glm/gtc/matrix_transform.hpp>
#include <array>

class AABB;
class CameraLens;
struct Ray;


class EditorCamera : public EventListener
{
public:

	EditorCamera();

	virtual ~EditorCamera();

	bool Awake();

	bool PreUpdate();

	bool CleanUp();

	void LockCamera(bool _lockCamera) { lockCamera = _lockCamera; }

	CameraLens* GetCameraLens() { return cameraLens; }
	
	bool GetCameraLocked() { return lockCamera; }

	//EVENTS
	void OnEvent(const Event& event) override;

private:
	void CalcMouseVectors();

public: 
	bool windowChanged;
	bool viewChanged;

private:
	
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

	CameraLens* cameraLens;
};