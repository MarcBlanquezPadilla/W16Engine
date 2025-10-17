#include "Module.h"

class OpenGL : public Module
{
public:

	OpenGL(bool startEnabled);

	virtual ~OpenGL();

	bool Awake();

	bool PreUpdate();
	bool PostUpdate();

	bool CleanUp();

private:
	unsigned int VAO;
	unsigned int shaderProgram;
};