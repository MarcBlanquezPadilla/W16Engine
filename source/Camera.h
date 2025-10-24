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

private:
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;

	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 up;
	glm::vec3 right;
};