#pragma once
#include "Module.h"
#include <glm/gtc/matrix_transform.hpp>
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>

struct MeshData;
struct Vertex;
class GameObject;

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
	
	bool UploadLinesToGPU(unsigned int& vao, unsigned int& vbo, const std::vector<glm::vec3>& lines);

	unsigned int UploadTextureToGPU(unsigned char* data, int width, int height);
	void DeleteTextureFromGPU(unsigned int textureID);

	bool RecursiveGameObjectsDraw(GameObject* gameObject, const glm::mat4& parentModelMatrix);

	void ChangeWindowSize(int x, int y);

	std::string GetGLVersion() { return glVersion; }
	std::string GetGLSLVersion() { return glslVersion; }
	std::string GetGPU() { return gpu; }


private:
	bool CreateDefaultShader();
	bool CreateCheckerTexture();
	bool CreateNormalShader();

private:
	unsigned int shaderProgram;
	unsigned int normalShaderProgram;
	unsigned int checkerTextureID;

	GLint modelMatrixLoc;
	GLint viewMatrixLoc;
	GLint projectionMatrixLoc;

	GLint normalModelMatrixLoc;
	GLint normalViewMatrixLoc;
	GLint normalProjectionMatrixLoc;

	GLint hasUVsLoc;

	std::string glVersion;
	std::string glslVersion;
	std::string devilVersion;
	std::string gpu;
};