#pragma once
#include "Module.h"
#include "EventListener.h"
#include "Global.h"
#include <glm/gtc/matrix_transform.hpp>
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <string>

struct MeshData;
struct StencilData;
struct Vertex;
class GameObject;
class CameraLens;
class MeshRenderer;

#define CHECKERS_WIDTH 64
#define CHECKERS_HEIGHT 64

struct RenderObject
{
	MeshRenderer* mesh;
	glm::mat4 globalModelMatrix;
};

struct RenderLine
{
	glm::vec3 startPoint;
	glm::vec3 endPoint;
	glm::vec4 color;
};

class ModuleRender : public Module, public EventListener
{
public:

	ModuleRender(bool startEnabled);

	~ModuleRender() override;

	bool Awake();

	bool PreUpdate();
	bool PostUpdate();

	bool CleanUp();

	void UpdateProjectionMatix(glm::mat4 projectionMatrix);
	void UpdateViewMatix(glm::mat4 viewMatrix);

	void CreateSharedShadersCode();
	static bool CreateShaderFromSources(unsigned int& shaderID, int type, const char* source, const int soruceLength);
	
	bool UploadMeshToGPU(MeshData& meshData, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
	void DeleteMeshFromGPU(MeshData& meshData);

	bool UploadSmoothedMeshToGPU(StencilData& stencilData, unsigned int& sharedEbo,const std::vector<Vertex>& vertices);
	void DeleteSmoothedMeshFromGPU(StencilData& stencilData);
	
	bool UploadLinesToGPU(unsigned int& vao, unsigned int& vbo, const std::vector<glm::vec3>& lines);

	unsigned int UploadTextureToGPU(unsigned char* data, int width, int height);
	void DeleteTextureFromGPU(unsigned int textureID);

	void CreateSkinningSSBOs(unsigned int& ssboGlobal, unsigned int& ssboOffset, const std::vector<glm::mat4>& offsets);
	void UploadGlobalMatricesToGPU(unsigned int ssbo, const std::vector<glm::mat4>& globalMatrices);
	void DeleteSSBO(unsigned int& ssbo);

	void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color);

	//WINDOW
	void ChangeWindowSize(int x, int y);

	//RENDER
	void AddMesh(MeshRenderer* mesh);
	void RemoveMesh(MeshRenderer* mesh);
	bool RenderScene(const CameraLens* camera);
	void AddCamera(CameraLens* camera);
	void RemoveCamera(CameraLens* camera);
	CameraLens* GetMainCamera();
	int GetMainCamerasNum() { return mainCameras; }

	//INFORMATION
	std::string GetGLVersion() { return glVersion; }
	std::string GetGLSLVersion() { return glslVersion; }
	std::string GetGPU() { return gpu; }

	//EVENTS
	void OnEvent(const Event& event) override;

	//GET OBJECT
	UID GetObjectInPixel(const CameraLens* camera, int x, int y);


private:

	//CREATE BASIC TEXTURES
	bool CreateDefaultTexture();
	bool CreateCheckerTexture();

	//CREATE SHADERS FUNCTIONS
	bool CreateDefaultShader();
	bool CreateNormalShader();
	bool CreateMeshLinesShader();
	bool CreateOutlineShader();
	bool CreateLineShader();
	bool CreatePickingShader();

	//DRAW FUNCTIONS
	void DrawRenderList(const std::multimap<float, RenderObject>& map, const CameraLens* camera);
	void DrawLinesList(const CameraLens* camera);
	void DrawStencilList(const CameraLens* camera);
	void DrawNormalsList(const CameraLens* camera);
	void DrawMeshLinesList(const CameraLens* camera);
	void BuildRenderLists(const CameraLens* camera);

private:

	glm::vec4 debugColor;
	glm::vec4 stencilColor;

	//SHADERS
	unsigned int uboMatrices;
	unsigned int ssboBones;

	//MODEL DRAW
	unsigned int shaderProgram;
	GLint modelMatrixLoc;
	GLint hasUVsLoc;
	GLint hasBonesLoc = 0;
	GLint meshInverseLoc = 0;

	//NORMAL DRAW
	unsigned int normalShaderProgram;
	GLint normalModelMatrixLoc;
	GLint normalColorLoc;
	GLint normalHasBonesLoc = 0;
	GLint normalMeshInverseLoc = 0;

	//STENCIL DRAW
	unsigned int outlineShaderProgram;
	GLint outlineModelMatrixLoc;
	GLint outlineColorLoc;
	GLint outlineHasBonesLoc = 0;
	GLint outlineMeshInverseLoc = 0;

	//LINES DRAW
	unsigned int lineShaderProgram;
	GLint lineModelMatrixLoc;
	GLint lineColorLoc;
	unsigned int lineVAO = 0;
	unsigned int lineVBO = 0;

	//MESH LINES DRAW
	unsigned int meshLinesShaderProgram;
	GLint meshLinesModelMatrixLoc;
	GLint meshLinesColorLoc;
	GLint meshLinesHasBonesLoc = 0;
	GLint meshLinesMeshInverseLoc = 0;

	//PICKING SHADER
	unsigned int pickingShaderProgram;
	GLint pickingModelMatrixLoc;
	GLint pickingHasUVsLoc;
	GLint pickingHasBonesLoc = 0;
	GLint pickingColorLoc;
	GLint pickingMeshInverseLoc = 0;

	std::string shaderHeader;
	std::string skinningDeclarations;
	std::string skinningFunction;

	unsigned int defaultTextureID;
	unsigned int checkerTextureID;

	std::string glVersion;
	std::string glslVersion;
	std::string devilVersion;
	std::string gpu;

	std::multimap<float,RenderObject> opaqueList;
	std::multimap<float,RenderObject> transparentList;
	std::vector<RenderObject> stencilList;
	std::vector<RenderObject> normalsList;
	std::vector<RenderObject> meshLinesList;
	std::vector<RenderLine> linesList;

	std::vector<MeshRenderer*> meshes;
	std::vector<CameraLens*> activeCameras;
	CameraLens* mainCamera;
	int mainCameras;
};