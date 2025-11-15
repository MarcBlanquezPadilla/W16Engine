#pragma once
#include "Module.h"
#include <glm/gtc/matrix_transform.hpp>
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <string>

struct MeshData;
struct Vertex;
class GameObject;
class Mesh;

#define CHECKERS_WIDTH 64
#define CHECKERS_HEIGHT 64

struct RenderObject
{
	Mesh* mesh;
	unsigned int textToBind;
	glm::mat4 globalModelMatrix;
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

	void UpdateProjectionMatix(glm::mat4 projectionMatrix);
	void UpdateViewMatix(glm::mat4 viewMatrix);

	static bool CreateShaderFromSources(unsigned int& shaderID, int type, const char* source, const int soruceLength);
	
	bool UploadMeshToGPU(MeshData& meshData, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
	void DeleteMeshFromGPU(MeshData& meshData);

	bool UploadSmoothedMeshToGPU(unsigned int& vao, unsigned int& vbo, unsigned int& sharedEbo,const std::vector<Vertex>& vertices);
	
	bool UploadLinesToGPU(unsigned int& vao, unsigned int& vbo, const std::vector<glm::vec3>& lines);

	unsigned int UploadTextureToGPU(unsigned char* data, int width, int height);
	void DeleteTextureFromGPU(unsigned int textureID);

	void BuildRenderListsRecursive(GameObject* gameObject);
	void DrawRenderList(const std::multimap<float,RenderObject>& map);

	void ChangeWindowSize(int x, int y);

	std::string GetGLVersion() { return glVersion; }
	std::string GetGLSLVersion() { return glslVersion; }
	std::string GetGPU() { return gpu; }


private:
	bool CreateDefaultShader();
	bool CreateCheckerTexture();
	bool CreateNormalShader();
	bool CreateOutlineShader();

private:
	unsigned int shaderProgram;
	unsigned int normalShaderProgram;
	unsigned int outlineShaderProgram;
	unsigned int checkerTextureID;

	GLint modelMatrixLoc;
	GLint viewMatrixLoc;
	GLint projectionMatrixLoc;

	GLint normalModelMatrixLoc;
	GLint normalViewMatrixLoc;
	GLint normalProjectionMatrixLoc;

	GLint outlineModelMatrixLoc;
	GLint outlineViewMatrixLoc;
	GLint outlineProjectionMatrixLoc;
	GLint outlineColorLoc;

	GLint hasUVsLoc;

	std::string glVersion;
	std::string glslVersion;
	std::string devilVersion;
	std::string gpu;

	std::multimap<float,RenderObject> opaqueList;
	std::multimap<float,RenderObject> transparentList;
};