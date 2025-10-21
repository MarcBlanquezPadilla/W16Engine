#include "Module.h"


class Render : public Module
{
public:

	Render(bool startEnabled);

	virtual ~Render();

	bool Awake();

	bool PreUpdate();
	bool PostUpdate();

	bool CleanUp();

	static bool CreateShaderFromSources(unsigned int& shaderID, int type, const char* source, const int soruceLength);

private:
	unsigned int VAO;
	unsigned int shaderProgram;
};