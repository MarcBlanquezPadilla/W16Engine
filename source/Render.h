#include "Module.h"
#include <glm/gtc/matrix_transform.hpp>

#define CHECKERS_WIDTH 64
#define CHECKERS_HEIGHT 64

class Render : public Module
{
public:

	Render(bool startEnabled);

	virtual ~Render();

	bool Awake();

	bool PreUpdate();
	bool PostUpdate();

	bool CleanUp();

	void UpdateProjectionMatix(glm::mat4 projectionMatrix);
	void UpdateViewMatix(glm::mat4 viewMatrix);

	static bool CreateShaderFromSources(unsigned int& shaderID, int type, const char* source, const int soruceLength);

private:
	unsigned int VAO;
	unsigned int shaderProgram;
	unsigned int textureID;
};