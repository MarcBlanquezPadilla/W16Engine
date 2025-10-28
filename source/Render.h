#pragma once
#include "Module.h"
#include <glm/gtc/matrix_transform.hpp>
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <vector>

#define CHECKERS_WIDTH 64
#define CHECKERS_HEIGHT 64

struct aiMesh;
struct aiScene;

struct Vertex
{
	glm::vec3 position;
};

struct Mesh
{
	unsigned int VAO = 0;
	unsigned int VBO = 0;
	unsigned int EBO = 0;
	int numIndices = 0;
};

class Render : public Module
{
public:

	Render(bool startEnabled);

	virtual ~Render();

	bool Awake();

	bool PreUpdate();
	bool PostUpdate();

	bool CleanUp();

	bool LoadModel(const std::string& filePath);

	void UpdateProjectionMatix(glm::mat4 projectionMatrix);
	void UpdateViewMatix(glm::mat4 viewMatrix);

	static bool CreateShaderFromSources(unsigned int& shaderID, int type, const char* source, const int soruceLength);
	
private:
	void ProcessAndUploadMesh(aiMesh* assimpMesh);
	bool CreateDefaultShader();
	bool CreateCheckerTexture();

private:
	unsigned int shaderProgram;
	unsigned int checkerTextureID;

	GLint modelMatrixLoc;
	GLint viewMatrixLoc;
	GLint projectionMatrixLoc;

	std::vector<Mesh> loadedMeshes;
};