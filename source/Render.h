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

struct RenderLine
{
	glm::vec3 startPoint;
	glm::vec3 endPoint;
	glm::vec4 color;
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



	void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color);

	void ChangeWindowSize(int x, int y);

	std::string GetGLVersion() { return glVersion; }
	std::string GetGLSLVersion() { return glslVersion; }
	std::string GetGPU() { return gpu; }


private:
	bool CreateDefaultShader();
	bool CreateCheckerTexture();
	bool CreateNormalShader();
	bool CreateOutlineShader();
	bool CreateLineShader();

	void DrawRenderList(const std::multimap<float, RenderObject>& map);
	void DrawLinesList(std::vector<RenderLine> list);
	void DrawStencil();
	void BuildRenderListsRecursive(GameObject* gameObject);

private:
	unsigned int shaderProgram;
	unsigned int normalShaderProgram;
	unsigned int outlineShaderProgram;
	unsigned int checkerTextureID;

	//MODEL DRAW
	GLint modelMatrixLoc;
	GLint viewMatrixLoc;
	GLint projectionMatrixLoc;

	//NORMAL DRAW
	GLint normalModelMatrixLoc;
	GLint normalViewMatrixLoc;
	GLint normalProjectionMatrixLoc;

	//STENCIL DRAW
	GLint outlineModelMatrixLoc;
	GLint outlineViewMatrixLoc;
	GLint outlineProjectionMatrixLoc;
	GLint outlineColorLoc;
	Mesh* selectedMesh;

	//LINES DRAW
	unsigned int lineShaderProgram;
	GLint lineModelMatrixLoc;
	GLint lineViewMatrixLoc;
	GLint lineProjectionMatrixLoc;
	GLint lineColorLoc;
	unsigned int lineVAO = 0;
	unsigned int lineVBO = 0;

	GLint hasUVsLoc;

	std::string glVersion;
	std::string glslVersion;
	std::string devilVersion;
	std::string gpu;

	std::multimap<float,RenderObject> opaqueList;
	std::multimap<float,RenderObject> transparentList;
	std::vector<RenderLine> linesList;
};