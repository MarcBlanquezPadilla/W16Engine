#pragma once
#include "Module.h"
#include <glm/gtc/matrix_transform.hpp>
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <vector>

struct MeshData;
struct Vertex;

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
	
	bool UploadMeshToGPU(MeshData& meshData, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
	void DeleteMeshFromGPU(MeshData& meshData);

	unsigned int UploadTextureToGPU(unsigned char* data, int width, int height);
	void DeleteTextureFromGPU(unsigned int textureID);

private:
	bool CreateDefaultShader();
	bool CreateCheckerTexture();

private:
	unsigned int shaderProgram;
	unsigned int checkerTextureID;

	GLint modelMatrixLoc;
	GLint viewMatrixLoc;
	GLint projectionMatrixLoc;
	GLint hasUVsLoc;

};