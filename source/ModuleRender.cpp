#include "glad/glad.h"
#include <IL/il.h>
#include <glm/gtc/type_ptr.hpp>
#include <SDL3/sdl.h>
#include <algorithm>

#include "Engine.h"
#include "ModuleEvents.h"
#include "ModuleRender.h"
#include "ModuleWindow.h"
#include "ModuleScene.h"

#include "CameraLens.h"

#include "GameObject.h"

#include "components/MeshRenderer.h"
#include "components/SkinnedMeshRenderer.h"

#include "resources/ResourceMesh.h"
#include "resources/ResourceTexture.h"

#include "utils/Frustum.h"
#include "utils/Log.h"

#include "geometry/Vertex.h"
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>


ModuleRender::ModuleRender(bool startEnabled) : Module(startEnabled)
{
	name = "Render";
}

ModuleRender::~ModuleRender()
{

}

bool ModuleRender::Awake()
{
	bool ret = true;

	int version = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress));

	if (version == 0)
	{
		LOG(LogType::LOG_ERROR, "Failed loading the glad library.");
		return false;
	}

	LOG(LogType::LOG_INFO, "Initializing Devil.");
	ilInit();
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
		
	LOG(LogType::LOG_INFO, "Initializing Glad.");
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glClearDepth(1.0f); 
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);  
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	glGenBuffers(1, &ssboBones);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboBones);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboBones);

	glGenBuffers(1, &uboMatrices);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboMatrices);

	gpu = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
	glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	glslVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

	//SHADERS
	CreateSharedShadersCode();

	//CREATE DEFAULT SHADER
	if (!CreateDefaultShader())
	{
		LOG(LogType::LOG_ERROR, "Failed creating default shader");
		return false;
	}

	//CREATE NORMAL SHADER
	if (!CreateNormalShader())
	{
		LOG(LogType::LOG_ERROR, "Failed creating normal shader");
		return false;
	}

	//CREATE STENCIL SHADER
	if (!CreateOutlineShader())
	{
		LOG(LogType::LOG_ERROR, "Failed creating outline shader");
		return false;
	}
	
	//CREATE LINE SHADER
	if (!CreateLineShader())
	{
		LOG(LogType::LOG_ERROR, "Failed creating outline shader");
		return false;
	}

	//CREATE MESH LINES SHADER
	if (!CreateMeshLinesShader())
	{
		LOG(LogType::LOG_ERROR, "Failed creating mesh lines shader");
		return false;
	}

	if (!CreatePickingShader())
	{
		LOG(LogType::LOG_ERROR, "Failed creating picking shader");
		return false;
	}

	//CREATE CHECKER TEXTURE
	if (!CreateDefaultTexture())
	{
		LOG(LogType::LOG_ERROR, "Failed creating default texture");
		return false;
	}

	//CREATE CHECKER TEXTURE
	if (!CreateCheckerTexture())
	{
		LOG(LogType::LOG_ERROR, "Failed creating checker texture");
		return false;
	}

	Engine::GetInstance().moduleEvents->Subscribe(Event::Type::WindowResize, this);

	debugColor = glm::vec4(DEBUG_COLOR);
	stencilColor = glm::vec4(STENCIL_COLOR);
	
	mainCamera = nullptr;
	mainCameras = 0;
	
	return ret;
}

bool ModuleRender::PreUpdate()
{
	bool ret = true;

	stencilList.clear();
	linesList.clear();
	normalsList.clear();
	meshLinesList.clear();
	mainCamera = nullptr;

	return ret;
}

bool ModuleRender::PostUpdate()
{
	bool ret = true;

	std::sort(activeCameras.begin(), activeCameras.end(), [](CameraLens* a, CameraLens* b) {

		return a->depth < b->depth;
	});

	for (CameraLens* camera : activeCameras)
	{
		RenderScene(camera);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Engine::GetInstance().moduleWindow->width, Engine::GetInstance().moduleWindow->height);

	glDisable(GL_SCISSOR_TEST);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	return ret;
}

bool ModuleRender::CleanUp()
{
	bool ret = true;
	
	meshes.clear();
	activeCameras.clear();
	mainCamera = nullptr;
	Engine::GetInstance().moduleEvents->UnsubscribeAll(this);

	glDeleteProgram(shaderProgram);
	glDeleteProgram(normalShaderProgram);
	glDeleteProgram(outlineShaderProgram);
	ilShutDown();

	return ret;
}

#pragma region Draw

bool ModuleRender::RenderScene(const CameraLens* camera)
{
	if (!camera) return false;

	//BIND FRAMEBUFFER
	if (camera->fboID != 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, camera->fboID);
		glViewport(0, 0, camera->textureWidth, camera->textureHeight);
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, Engine::GetInstance().moduleWindow->width, Engine::GetInstance().moduleWindow->height);
	}

	//CLEAN BUFFERS
	glDisable(GL_SCISSOR_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearStencil(0);

	//UPDATE CAM MATRIX
	UpdateViewMatix(camera->GetViewMatrix());
	UpdateProjectionMatix(camera->GetProjectionMatrix());

	//CLEAN LIST AND BUILD NEWS
	opaqueList.clear();
	transparentList.clear();

	BuildRenderLists(camera);

	//CONFIG OPENGL
	glUseProgram(shaderProgram);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	//RENDER OPAQUES
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	DrawRenderList(opaqueList, camera);

	//RENDER TRANSPARENT
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	DrawRenderList(transparentList, camera);

	//RENDER DEBUG
	if (camera->GetDebugCamera())
	{
		DrawStencilList(camera);
		DrawNormalsList(camera);
		DrawLinesList(camera);
		DrawMeshLinesList(camera);
	}

	//RESET STATES
	glDisable(GL_STENCIL_TEST);
	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	//UNBINF FRAMEBUFFER
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void ModuleRender::BuildRenderLists(const CameraLens* camera)
{
	for (MeshRenderer* mesh : meshes)
	{
		if (!mesh->GetEnabled()) continue;

		ResourceMesh* resMesh = mesh->GetMeshResource();
		if (!resMesh || !resMesh->IsLoadedToMemory()) continue;

		glm::mat4 globalModelMatrix;
		mesh->owner->GetGlobalMatrix(globalModelMatrix);

		mesh->UpdateDynamicAABB();
		const AABB& globalAABB = mesh->GetGlobalAABB();

		if (camera->GetFrustum()->InFrustum(globalAABB))
		{
			mesh->UpdateSkinningMatrices();

			RenderObject renderObject = { mesh, globalModelMatrix };

			glm::vec3 aabbCenter = (globalAABB.min + globalAABB.max) * 0.5f;
			float distanceToCamera = glm::distance(aabbCenter, camera->position);

			if (mesh->GetTransparent())
			{
				transparentList.emplace(distanceToCamera, renderObject);
			}
			else
			{
				opaqueList.emplace(distanceToCamera, renderObject);
			}

			if (mesh->drawNormals) normalsList.push_back(renderObject);
			if (mesh->drawMesh) meshLinesList.push_back(renderObject);
		}
	}
}

void ModuleRender::DrawRenderList(const std::multimap<float, RenderObject>& map, const CameraLens* camera)
{
	for (auto pair = map.rbegin(); pair != map.rend(); ++pair)
	{
		RenderObject renderObject = pair->second;
		MeshRenderer* meshComp = renderObject.mesh;

		if (meshComp->drawStencil) {
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glStencilMask(0xFF);
			stencilList.push_back(renderObject);
		}
		else {
			glStencilFunc(GL_ALWAYS, 0, 0xFF);
			glStencilMask(0x00);
		}

		unsigned int texToBind = defaultTextureID;
		const ResourceTexture* resTex = meshComp->GetTextureResource();

		if (meshComp->drawChecker) {
			texToBind = checkerTextureID;
		}
		else if (resTex && resTex->IsLoadedToMemory() && resTex->GetTextureGpuId() != 0) {
			texToBind = resTex->GetTextureGpuId();
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texToBind);
		glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, glm::value_ptr(renderObject.globalModelMatrix));
		glUniform1i(hasUVsLoc, true);

		if (meshComp->HasSkinning())
		{
			SkinnedMeshRenderer* skinnedMeshComp = (SkinnedMeshRenderer*)meshComp;

			glUniformMatrix4fv(meshInverseLoc, 1, GL_FALSE, glm::value_ptr(skinnedMeshComp->GetMeshInverse()));

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, skinnedMeshComp->GetSSBOGlobal());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, skinnedMeshComp->GetSSBOOffset());

			glUniform1i(hasBonesLoc, true);
		}
		else
		{
			glUniform1i(hasBonesLoc, false);
		}

		glBindVertexArray(meshComp->GetMeshResource()->meshData.VAO);
		glDrawElements(GL_TRIANGLES, meshComp->GetMeshResource()->numIndices, GL_UNSIGNED_INT, 0);
	}
}

void ModuleRender::DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color)
{
	RenderLine line = { start, end, color };
	linesList.push_back(line);
}

void ModuleRender::DrawLinesList(const CameraLens* camera)
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	for (RenderLine line : linesList)
	{
		glUseProgram(lineShaderProgram);

		glm::mat4 model = glm::mat4(1.0f);
		glUniformMatrix4fv(lineModelMatrixLoc, 1, GL_FALSE, glm::value_ptr(model));

		glUniform4fv(lineColorLoc, 1, glm::value_ptr(line.color));

		glm::vec3 vertices[2] = { line.startPoint, line.endPoint };
		glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(lineVAO);
		glDrawArrays(GL_LINES, 0, 2);
		glBindVertexArray(0);

		glUseProgram(0);
	}
}

void ModuleRender::DrawNormalsList(const CameraLens* camera)
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	
	//DRAW NORMALS
	for (RenderObject renderObject : normalsList)
	{
		if (renderObject.mesh->drawNormals && renderObject.mesh->GetMeshResource()->meshData.VAO != 0)
		{
			glUseProgram(normalShaderProgram);
			glUniformMatrix4fv(normalModelMatrixLoc, 1, GL_FALSE, glm::value_ptr(renderObject.globalModelMatrix));

			glUniform4f(normalColorLoc, debugColor.r, debugColor.g, debugColor.b, debugColor.a);

			MeshRenderer* meshComp = renderObject.mesh;

			if (meshComp->HasSkinning())
			{
				SkinnedMeshRenderer* skinnedMeshComp = (SkinnedMeshRenderer*)meshComp;

				glUniformMatrix4fv(normalMeshInverseLoc, 1, GL_FALSE, glm::value_ptr(skinnedMeshComp->GetMeshInverse()));

				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, skinnedMeshComp->GetSSBOGlobal());
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, skinnedMeshComp->GetSSBOOffset());

				glUniform1i(normalHasBonesLoc, true);
			}
			else
			{
				glUniform1i(normalHasBonesLoc, false);
			}

			glBindVertexArray(renderObject.mesh->GetMeshResource()->meshData.VAO);

			glDrawArrays(GL_POINTS, 0, renderObject.mesh->GetMeshResource()->numVertices);

			glUseProgram(shaderProgram);
		}
	}
}

void ModuleRender::DrawStencilList(const CameraLens* camera)
{
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (RenderObject renderObject : stencilList)
	{
		glUseProgram(outlineShaderProgram);

		glUniformMatrix4fv(outlineModelMatrixLoc, 1, GL_FALSE, glm::value_ptr(renderObject.globalModelMatrix));
		glUniform4f(outlineColorLoc, stencilColor.r, stencilColor.g, stencilColor.b, stencilColor.a);

		MeshRenderer* meshComp = renderObject.mesh;

		if (meshComp->HasSkinning())
		{
			SkinnedMeshRenderer* skinnedMeshComp = (SkinnedMeshRenderer*)meshComp;

			glUniformMatrix4fv(outlineMeshInverseLoc, 1, GL_FALSE, glm::value_ptr(skinnedMeshComp->GetMeshInverse()));

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, skinnedMeshComp->GetSSBOGlobal());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, skinnedMeshComp->GetSSBOOffset());

			glUniform1i(outlineHasBonesLoc, true);
		}
		else
		{
			glUniform1i(outlineHasBonesLoc, false);
		}

		glBindVertexArray(renderObject.mesh->GetMeshResource()->stencilData.VAO);

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE);

		glDrawElements(GL_TRIANGLES, renderObject.mesh->GetMeshResource()->numIndices, GL_UNSIGNED_INT, 0);

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glStencilFunc(GL_ALWAYS, 2, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);
		glDepthFunc(GL_LEQUAL);
		glDrawElements(GL_TRIANGLES, renderObject.mesh->GetMeshResource()->numIndices, GL_UNSIGNED_INT, 0);

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthFunc(GL_GREATER);
		glStencilFunc(GL_NOTEQUAL, 2, 0xFF);
		glStencilMask(0x00);

		glDrawElements(GL_TRIANGLES, renderObject.mesh->GetMeshResource()->numIndices, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
		glUseProgram(0);
	}

	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glDisable(GL_STENCIL_TEST);
}

void ModuleRender::DrawMeshLinesList(const CameraLens* camera)
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	for (RenderObject renderObject : meshLinesList)
	{
		glUseProgram(meshLinesShaderProgram);

		glUniformMatrix4fv(meshLinesModelMatrixLoc, 1, GL_FALSE, glm::value_ptr(renderObject.globalModelMatrix));
		glUniform4f(meshLinesColorLoc, debugColor.r, debugColor.g, debugColor.b, debugColor.a);

		MeshRenderer* meshComp = renderObject.mesh;
		if (meshComp->HasSkinning())
		{
			SkinnedMeshRenderer* skinnedMeshComp = (SkinnedMeshRenderer*)meshComp;

			glUniformMatrix4fv(meshLinesMeshInverseLoc, 1, GL_FALSE, glm::value_ptr(skinnedMeshComp->GetMeshInverse()));

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, skinnedMeshComp->GetSSBOGlobal());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, skinnedMeshComp->GetSSBOOffset());

			glUniform1i(meshLinesHasBonesLoc, true);
		}
		else
		{
			glUniform1i(meshLinesHasBonesLoc, false);
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(-1.0f, -1.0f);

		glBindVertexArray(renderObject.mesh->GetMeshResource()->meshData.VAO);
		glDrawElements(GL_TRIANGLES, renderObject.mesh->GetMeshResource()->numIndices, GL_UNSIGNED_INT, 0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_POLYGON_OFFSET_LINE);
		glBindVertexArray(0);
		glUseProgram(0);
	}
}

#pragma endregion

#pragma region Shaders

void ModuleRender::CreateSharedShadersCode()
{
	shaderHeader =
		"#version 460 core\n"
		"layout(std140, binding = 0) uniform Matrices {\n"
		"    mat4 view;\n"
		"    mat4 projection;\n"
		"};\n";

	skinningDeclarations =
		"layout(std430, binding = 0) readonly buffer BoneMatrices { mat4 gBones[]; };\n"
		"layout(std430, binding = 1) readonly buffer OffsetMatrices { mat4 gOffsets[]; };\n"
		"uniform mat4 meshInverse;\n"
		"uniform bool hasBones;\n"
		"uniform mat4 model;\n";

	skinningFunction =
		"mat4 GetSkinMatrix(ivec4 ids, vec4 weights) {\n"
		"    if (!hasBones) return mat4(1.0);\n"
		"    mat4 skinMat = mat4(0.0);\n"
		"    float weightSum = weights.x + weights.y + weights.z + weights.w;\n"
		"    if (weightSum < 0.001) return mat4(1.0);\n"
		"    for(int i = 0; i < 4; i++) {\n"
		"        if(ids[i] == -1) continue;\n"
		"        // Calculamos la matriz de este hueso específico\n"
		"        mat4 boneTransform = meshInverse * gBones[ids[i]] * gOffsets[ids[i]];\n"
		"        // La sumamos pesada por su influencia (weight)\n"
		"        skinMat += boneTransform * (weights[i] / weightSum);\n"
		"    }\n"
		"    return skinMat;\n"
		"}\n";
}

bool ModuleRender::CreateShaderFromSources(unsigned int& shaderID, int type, const char* source, const int soruceLength)
{
	shaderID = glCreateShader(type);
	glShaderSource(shaderID, 1, &source, &soruceLength);
	glCompileShader(shaderID);

	int status = 0;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		int length = 0;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length);
		if (length > 0)
		{
			char* logg = new char[length];
			glGetShaderInfoLog(shaderID, length, nullptr, logg);
			LOG(LogType::LOG_ERROR, "%s", logg);
			delete[] logg;
		}
		return false;
	}
	return true;
}

bool ModuleRender::CreateDefaultShader()
{
	unsigned int vShader = 0;
	std::string vSource = shaderHeader + skinningDeclarations + skinningFunction +
		"layout (location = 0) in vec3 position;\n"
		"layout (location = 1) in vec2 aTexCoord;\n"
		"layout (location = 3) in ivec4 boneIDs;\n"
		"layout (location = 4) in vec4 weights;\n"
		"\n"
		"out vec3 localPos;\n"
		"out vec2 texCoord;\n\n"
		"void main()\n"
		"{\n"
		"    // Usamos la función de la librería compartida\n"
		"    mat4 skinMat = GetSkinMatrix(boneIDs, weights);\n"
		"    vec4 skinnedPos = skinMat * vec4(position, 1.0f);\n"
		"\n"
		"    // Proyección final\n"
		"    gl_Position = projection * view * model * skinnedPos;\n"
		"\n"
		"    localPos = position;\n"
		"    texCoord = aTexCoord;\n"
		"}\n";

	if (!CreateShaderFromSources(vShader, GL_VERTEX_SHADER, vSource.c_str(), vSource.length()))
		return false;

	unsigned int fShader = 0;
	const char* fragmentShaderSource = "#version 460 core\n"
		"in vec3 localPos;\n"
		"in vec2 texCoord;\n"
		"out vec4 color;\n"
		"uniform sampler2D texture1;\n"
		"uniform bool hasUVs;\n"
		"void main()\n"
		"{\n"
		"    vec2 uv = texCoord;\n"
		"    if (!hasUVs)\n"
		"    {\n"
		"        uv = localPos.xz * 0.5; \n"
		"    }\n"
		"    color = texture(texture1, uv);\n"
		"}\n";

	if (!CreateShaderFromSources(fShader, GL_FRAGMENT_SHADER, fragmentShaderSource, strlen(fragmentShaderSource)))
		return false;

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vShader);
	glAttachShader(shaderProgram, fShader);
	glLinkProgram(shaderProgram);
	int status = 0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		int length = 0;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &length);
		if (length > 0)
		{
			char* logg = new char[length];
			glGetProgramInfoLog(shaderProgram, length, nullptr, logg);
			LOG(LogType::LOG_ERROR, "%s", logg);
			delete[] logg;
		}
		return false;
	}
	glDeleteShader(vShader);
	glDeleteShader(fShader);

	modelMatrixLoc = glGetUniformLocation(shaderProgram, "model");
	hasUVsLoc = glGetUniformLocation(shaderProgram, "hasUVs");
	hasBonesLoc = glGetUniformLocation(shaderProgram, "hasBones");
	meshInverseLoc = glGetUniformLocation(shaderProgram, "meshInverse");

	return true;
}

bool ModuleRender::CreateNormalShader()
{
	unsigned int vShader = 0;
	std::string vSource = shaderHeader + skinningDeclarations + skinningFunction +
		"layout (location = 0) in vec3 position;\n"
		"layout (location = 2) in vec3 aNormal;\n"
		"layout (location = 3) in ivec4 boneIDs;\n"
		"layout (location = 4) in vec4 weights;\n"
		"\n"
		"out VS_OUT { vec3 normal; } vs_out;\n"
		"\n"
		"void main() {\n"
		"    mat4 skinMat = GetSkinMatrix(boneIDs, weights);\n"
		"\n"
		"    vec4 skinnedPos = skinMat * vec4(position, 1.0f);\n"
		"    vec3 skinnedNormal = mat3(skinMat) * aNormal;\n"
		"\n"
		"    gl_Position = skinnedPos;\n"
		"    vs_out.normal = normalize(skinnedNormal);\n"
		"}\n";
	if (!CreateShaderFromSources(vShader, GL_VERTEX_SHADER, vSource.c_str(), vSource.length())) return false;

	unsigned int gShader = 0;
	std::string gSource = shaderHeader +
		"layout (points) in;\n"
		"layout (line_strip, max_vertices = 2) out;\n"
		"in VS_OUT { vec3 normal; } gs_in[];\n"
		"uniform mat4 model;\n"
		"const float LINE_LENGTH = 0.2;\n"
		"\n"
		"void main() {\n"
		"   vec3 worldNormal = normalize(mat3(transpose(inverse(model))) * gs_in[0].normal);\n"
		"   vec4 worldPos = model * gl_in[0].gl_Position;\n"
		"\n"
		"   gl_Position = projection * view * worldPos;\n"
		"   EmitVertex();\n"
		"\n"
		"   gl_Position = projection * view * (worldPos + vec4(worldNormal * LINE_LENGTH, 0.0));\n"
		"   EmitVertex();\n"
		"\n"
		"   EndPrimitive();\n"
		"}\n";
	if (!CreateShaderFromSources(gShader, GL_GEOMETRY_SHADER, gSource.c_str(), gSource.length())) return false;


	unsigned int fShader = 0;
	const char* fragmentSource = "#version 460 core\n"
		"out vec4 color;\n"
		"uniform vec4 lineColor;\n"
		"void main() { color = lineColor; }\n";
	if (!CreateShaderFromSources(fShader, GL_FRAGMENT_SHADER, fragmentSource, strlen(fragmentSource))) return false;

	normalShaderProgram = glCreateProgram();
	glAttachShader(normalShaderProgram, vShader);
	glAttachShader(normalShaderProgram, gShader);
	glAttachShader(normalShaderProgram, fShader);
	glLinkProgram(normalShaderProgram);

	glDeleteShader(vShader);
	glDeleteShader(gShader);
	glDeleteShader(fShader);

	normalModelMatrixLoc = glGetUniformLocation(normalShaderProgram, "model");
	normalColorLoc = glGetUniformLocation(normalShaderProgram, "lineColor");
	normalHasBonesLoc = glGetUniformLocation(normalShaderProgram, "hasBones");
	normalMeshInverseLoc = glGetUniformLocation(normalShaderProgram, "meshInverse");

	return true;
}

bool ModuleRender::CreateOutlineShader()
{
	unsigned int vShader = 0;
	std::string vSource = shaderHeader + skinningDeclarations + skinningFunction +
		"layout (location = 0) in vec3 position;\n"
		"layout (location = 2) in vec3 aNormal;\n"
		"layout (location = 3) in ivec4 boneIDs;\n"
		"layout (location = 4) in vec4 weights;\n"
		"\n"
		"uniform float u_outlineThickness = 0.03;\n"
		"\n"
		"void main() {\n"
		"    mat4 skinMat = GetSkinMatrix(boneIDs, weights);\n"
		"    vec4 skinnedPos = skinMat * vec4(position, 1.0);\n"
		"    vec3 skinnedNormal = mat3(skinMat) * aNormal;\n"
		"\n"
		"    vec3 worldNormal = normalize(mat3(model) * skinnedNormal);\n"
		"    vec4 worldPos = model * skinnedPos;\n"
		"\n"
		"    // Pasamos la posición al espacio de vista (View Space)\n"
		"    vec4 viewPos = view * worldPos;\n"
		"    float dist = length(viewPos.xyz);\n"
		"    \n"
		"    float dynamicThickness = u_outlineThickness * (dist * 0.1);\n"
		"    \n"
		"    worldPos.xyz += worldNormal * dynamicThickness;\n"
		"\n"
		"    gl_Position = projection * view * worldPos;\n"
		"}\n";

	if (!CreateShaderFromSources(vShader, GL_VERTEX_SHADER, vSource.c_str(), vSource.length()))
		return false;

	unsigned int fShader = 0;
	const char* fragmentShaderSource = "#version 460 core\n"
		"out vec4 color;\n"
		"uniform vec4 outlineColor;\n"
		"void main() { color = outlineColor; }\n";

	if (!CreateShaderFromSources(fShader, GL_FRAGMENT_SHADER, fragmentShaderSource, strlen(fragmentShaderSource)))
		return false;

	outlineShaderProgram = glCreateProgram();
	glAttachShader(outlineShaderProgram, vShader);
	glAttachShader(outlineShaderProgram, fShader);
	glLinkProgram(outlineShaderProgram);

	int status = 0;
	glGetProgramiv(outlineShaderProgram, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		LOG(LogType::LOG_ERROR, "Failed linking outline shader!");
		return false;
	}

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	outlineModelMatrixLoc = glGetUniformLocation(outlineShaderProgram, "model");
	outlineColorLoc = glGetUniformLocation(outlineShaderProgram, "outlineColor");
	outlineHasBonesLoc = glGetUniformLocation(outlineShaderProgram, "hasBones");
	outlineMeshInverseLoc = glGetUniformLocation(outlineShaderProgram, "meshInverse");

	return true;
}

bool ModuleRender::CreateLineShader()
{
	const char* vsSource = "#version 460 core\n"
		"layout (location = 0) in vec3 position;\n"
		"layout(std140, binding = 0) uniform Matrices {\n"
		"mat4 view;\n"
		"mat4 projection;\n"
		"};\n"
		"\n"
		"uniform mat4 model;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = projection * view * model * vec4(position, 1.0f);\n"
		"}\n";

	const char* fsSource = "#version 460 core\n"
		"out vec4 color;\n"
		"uniform vec4 lineColor;\n"
		"void main() { color = lineColor; }\n";

	unsigned int vShader = 0, fShader = 0;
	if (!CreateShaderFromSources(vShader, GL_VERTEX_SHADER, vsSource, strlen(vsSource))) return false;
	if (!CreateShaderFromSources(fShader, GL_FRAGMENT_SHADER, fsSource, strlen(fsSource))) return false;

	lineShaderProgram = glCreateProgram();
	glAttachShader(lineShaderProgram, vShader);
	glAttachShader(lineShaderProgram, fShader);
	glLinkProgram(lineShaderProgram);

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	lineModelMatrixLoc = glGetUniformLocation(lineShaderProgram, "model");
	lineColorLoc = glGetUniformLocation(lineShaderProgram, "lineColor");

	glGenVertexArrays(1, &lineVAO);
	glGenBuffers(1, &lineVBO);

	glBindVertexArray(lineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);

	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

bool ModuleRender::CreateMeshLinesShader()
{
	unsigned int vShader = 0;
	std::string vSource = shaderHeader + skinningDeclarations + skinningFunction +
		"layout (location = 0) in vec3 position;\n"
		"layout (location = 3) in ivec4 boneIDs;\n"
		"layout (location = 4) in vec4 weights;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    // Obtenemos la matriz de skinning unificada\n"
		"    mat4 skinMat = GetSkinMatrix(boneIDs, weights);\n"
		"    vec4 skinnedPos = skinMat * vec4(position, 1.0f);\n"
		"\n"
		"    // Proyección usando el UBO de cámara (binding 0)\n"
		"    gl_Position = projection * view * model * skinnedPos;\n"
		"}\n";

	if (!CreateShaderFromSources(vShader, GL_VERTEX_SHADER, vSource.c_str(), vSource.length()))
		return false;

	unsigned int fShader = 0;
	const char* fragmentShaderSource ="#version 460 core\n"
		"out vec4 color;\n"
		"uniform vec4 lineColor;\n"
		"void main() { color = lineColor; }\n";

	if (!CreateShaderFromSources(fShader, GL_FRAGMENT_SHADER, fragmentShaderSource, strlen(fragmentShaderSource)))
		return false;

	meshLinesShaderProgram = glCreateProgram();
	glAttachShader(meshLinesShaderProgram, vShader);
	glAttachShader(meshLinesShaderProgram, fShader);
	glLinkProgram(meshLinesShaderProgram);

	int status = 0;
	glGetProgramiv(meshLinesShaderProgram, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		LOG(LogType::LOG_ERROR, "Failed linking normal sahder.");
		return false;
	}

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	meshLinesModelMatrixLoc = glGetUniformLocation(meshLinesShaderProgram, "model");
	meshLinesColorLoc = glGetUniformLocation(meshLinesShaderProgram, "lineColor");
	meshLinesHasBonesLoc = glGetUniformLocation(meshLinesShaderProgram, "hasBones");
	meshLinesMeshInverseLoc = glGetUniformLocation(meshLinesShaderProgram, "meshInverse");

	return true;
}

bool ModuleRender::CreatePickingShader()
{
	unsigned int vShader = 0;
	std::string vSource = shaderHeader + skinningDeclarations + skinningFunction +
		"layout (location = 0) in vec3 position;\n"
		"layout (location = 1) in vec2 aTexCoord;\n"
		"layout (location = 3) in ivec4 boneIDs;\n"
		"layout (location = 4) in vec4 weights;\n"
		"\n"
		"out vec3 localPos;\n"
		"out vec2 texCoord;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    // Obtenemos la matriz de skinning unificada\n"
		"    mat4 skinMat = GetSkinMatrix(boneIDs, weights);\n"
		"    vec4 skinnedPos = skinMat * vec4(position, 1.0f);\n"
		"\n"
		"    // Proyección usando el UBO (binding 0)\n"
		"    gl_Position = projection * view * model * skinnedPos;\n"
		"\n"
		"    localPos = position;\n"
		"    texCoord = aTexCoord;\n"
		"}\n";

	if (!CreateShaderFromSources(vShader, GL_VERTEX_SHADER, vSource.c_str(), vSource.length()))
		return false;

	unsigned int fShader = 0;
	const char* fragmentShaderSource = "#version 460 core\n"
		"out vec4 color;\n"
		"uniform vec4 pickingColor;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    color = pickingColor;\n"
		"}\n";;

	if (!CreateShaderFromSources(fShader, GL_FRAGMENT_SHADER, fragmentShaderSource, strlen(fragmentShaderSource)))
		return false;

	pickingShaderProgram = glCreateProgram();
	glAttachShader(pickingShaderProgram, vShader);
	glAttachShader(pickingShaderProgram, fShader);
	glLinkProgram(pickingShaderProgram);
	int status = 0;
	glGetProgramiv(pickingShaderProgram, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		int length = 0;
		glGetProgramiv(pickingShaderProgram, GL_INFO_LOG_LENGTH, &length);
		if (length > 0)
		{
			char* logg = new char[length];
			glGetProgramInfoLog(pickingShaderProgram, length, nullptr, logg);
			LOG(LogType::LOG_ERROR, "%s", logg);
			delete[] logg;
		}
		return false;
	}
	glDeleteShader(vShader);
	glDeleteShader(fShader);

	pickingModelMatrixLoc = glGetUniformLocation(pickingShaderProgram, "model");
	pickingHasUVsLoc = glGetUniformLocation(pickingShaderProgram, "hasUVs");
	pickingHasBonesLoc = glGetUniformLocation(pickingShaderProgram, "hasBones");
	pickingMeshInverseLoc = glGetUniformLocation(pickingShaderProgram, "meshInverse");

	pickingColorLoc = glGetUniformLocation(pickingShaderProgram, "pickingColor");

	return true;
}

#pragma endregion

#pragma region Matrix
void ModuleRender::UpdateProjectionMatix(glm::mat4 pm) {
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(pm));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ModuleRender::UpdateViewMatix(glm::mat4 vm) {
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(vm));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

#pragma endregion

#pragma region GPU

bool ModuleRender::UploadMeshToGPU(MeshData& meshData, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
{
	//CREATE VAO
	glGenVertexArrays(1, &meshData.VAO);
	glBindVertexArray(meshData.VAO);

	//CREATE VBO
	glGenBuffers(1, &meshData.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, meshData.VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	//CREATE EBO
	glGenBuffers(1, &meshData.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, boneIDs));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));


	glBindVertexArray(0);

	LOG(LogType::LOG_INFO, "Mesh uploaded to GPU. VAO: %u, VBO: %u, EBO: %u, Indices: %d",
		meshData.VAO, meshData.VBO, meshData.EBO, indices.size());

	return true;

}

bool ModuleRender::UploadSmoothedMeshToGPU(StencilData& stencilData, unsigned int& sharedEbo, const std::vector<Vertex>& vertices)
{
	glGenVertexArrays(1, &stencilData.VAO);
	glBindVertexArray(stencilData.VAO);
	glGenBuffers(1, &stencilData.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, stencilData.VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sharedEbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, boneIDs));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	LOG(LogType::LOG_INFO, "Outline smoothed mesh upload to GPU. VAO: %u, VBO: % u", stencilData.VAO, stencilData.VBO);
	return true;
}

bool ModuleRender::UploadLinesToGPU(unsigned int& vao, unsigned int& vbo, const std::vector<glm::vec3>& lines)
{
	//CREATE VAO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//CREATE VBO
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(glm::vec3), &lines[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	LOG(LogType::LOG_INFO, "Lines of normals uploaded to GPU. VAO: %u, Vertices: %d", vao, lines.size());
	return true;
}

void ModuleRender::DeleteMeshFromGPU(MeshData& meshData)
{
	LOG(LogType::LOG_INFO, "Mesh removed from GPU. VAO: %d, EBO: %d, VBO: %d", meshData.VAO, meshData.EBO, meshData.VBO);
	if (meshData.VBO != 0) glDeleteBuffers(1, &meshData.VBO);
	if (meshData.EBO != 0) glDeleteBuffers(1, &meshData.EBO);
	if (meshData.VAO != 0) glDeleteVertexArrays(1, &meshData.VAO);
	meshData = MeshData();
}

void ModuleRender::DeleteSmoothedMeshFromGPU(StencilData& stencilData)
{
	LOG(LogType::LOG_INFO, "Mesh removed from GPU. VAO: %d, VBO: %d", stencilData.VAO, stencilData.VBO);
	if (stencilData.VBO != 0) glDeleteBuffers(1, &stencilData.VBO);
	if (stencilData.VAO != 0) glDeleteVertexArrays(1, &stencilData.VAO);
	stencilData = StencilData();
}

unsigned int ModuleRender::UploadTextureToGPU(unsigned char* data, int width, int height)
{
	unsigned int textureID = 0;

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	LOG(LogType::LOG_INFO, "Texture uploaded to GPU. ID: %u", textureID);
	return textureID;
}

void ModuleRender::DeleteTextureFromGPU(unsigned int textureID)
{
	if (textureID != 0)
	{
		glDeleteTextures(1, &textureID);
		LOG(LogType::LOG_INFO, "Texture removed from GPU. ID: %u", textureID);
	}
}

void ModuleRender::CreateSkinningSSBOs(unsigned int& ssboGlobal, unsigned int& ssboOffset, const std::vector<glm::mat4>& offsets)
{
	size_t numBones = offsets.size();

	if (ssboOffset == 0) glGenBuffers(1, &ssboOffset);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboOffset);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numBones * sizeof(glm::mat4), offsets.data(), GL_STATIC_DRAW);

	if (ssboGlobal == 0) glGenBuffers(1, &ssboGlobal);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboGlobal);

	glBufferData(GL_SHADER_STORAGE_BUFFER, numBones * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ModuleRender::UploadGlobalMatricesToGPU(unsigned int ssbo, const std::vector<glm::mat4>& globalMatrices)
{
	if (ssbo == 0) return;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, globalMatrices.size() * sizeof(glm::mat4), globalMatrices.data());
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ModuleRender::DeleteSSBO(unsigned int& ssbo)
{
	if (ssbo != 0)
	{
		glDeleteBuffers(1, &ssbo);
		ssbo = 0;
	}
}

#pragma endregion

#pragma region Cameras

void ModuleRender::AddCamera(CameraLens* camera)
{
	activeCameras.push_back(camera);
}

void ModuleRender::RemoveCamera(CameraLens* camera)
{
	auto it = std::remove(activeCameras.begin(), activeCameras.end(), camera);
	activeCameras.erase(it, activeCameras.end());
}

CameraLens* ModuleRender::GetMainCamera()
{
	if (mainCamera != nullptr && mainCamera->GetActiveCamera() && mainCamera->depth == 0)
	{
		return mainCamera;
	}

	mainCameras = 0;

	for (CameraLens* cam : activeCameras)
	{
		if (cam->GetActiveCamera() && cam->depth == 0)
		{
			mainCameras++;
			mainCamera = cam;
		}
	}

	if (mainCameras > 1)
	{
		LOG(LogType::LOG_WARNING, "There's more than one active camera with Depth = 0 (Main Layer)!");
	}

	mainCamera = nullptr;

	for (CameraLens* cam : activeCameras)
	{
		if (cam->GetActiveCamera() && cam->depth == 0)
		{
			mainCamera = cam;
			break;
		}
	}

	return mainCamera;
}

#pragma endregion

#pragma region Meshes

void ModuleRender::AddMesh(MeshRenderer* mesh) {
	meshes.push_back(mesh);
}

void ModuleRender::RemoveMesh(MeshRenderer* mesh) {
	auto it = std::find(meshes.begin(), meshes.end(), mesh);
	if (it != meshes.end()) {
		*it = meshes.back();
		meshes.pop_back();
	}
}

#pragma endregion

#pragma region Textures
bool ModuleRender::CreateCheckerTexture()
{
	GLubyte checkerImage[CHECKERS_HEIGHT][CHECKERS_WIDTH][4];
	for (int i = 0; i < CHECKERS_HEIGHT; i++) {
		for (int j = 0; j < CHECKERS_WIDTH; j++) {
			int c = ((((i & 0x8) == 0) ^ (((j & 0x8)) == 0))) * 255;
			checkerImage[i][j][0] = (GLubyte)c;
			checkerImage[i][j][1] = (GLubyte)c;
			checkerImage[i][j][2] = (GLubyte)c;
			checkerImage[i][j][3] = (GLubyte)255;
		}
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &checkerTextureID);
	glBindTexture(GL_TEXTURE_2D, checkerTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CHECKERS_WIDTH, CHECKERS_HEIGHT,
		0, GL_RGBA, GL_UNSIGNED_BYTE, checkerImage);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

bool ModuleRender::CreateDefaultTexture()
{
	GLubyte defaultImage[CHECKERS_HEIGHT][CHECKERS_WIDTH][4];
	for (int i = 0; i < CHECKERS_HEIGHT; i++) {
		for (int j = 0; j < CHECKERS_WIDTH; j++) {
			int c = ((((i & 0x8) == 0) ^ (((j & 0x8)) == 0))) * 255;
			defaultImage[i][j][0] = (GLubyte)200;
			defaultImage[i][j][1] = (GLubyte)200;
			defaultImage[i][j][2] = (GLubyte)200;
			defaultImage[i][j][3] = (GLubyte)255;
		}
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &defaultTextureID);
	glBindTexture(GL_TEXTURE_2D, defaultTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CHECKERS_WIDTH, CHECKERS_HEIGHT,
		0, GL_RGBA, GL_UNSIGNED_BYTE, defaultImage);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}
#pragma endregion

UID ModuleRender::GetObjectInPixel(const CameraLens* camera, int x, int y)
{
	if (!camera) return 0;

	glBindFramebuffer(GL_FRAMEBUFFER, (camera->fboID != 0) ? camera->fboID : 0);
	glViewport(0, 0, (camera->fboID != 0) ? camera->textureWidth : Engine::GetInstance().moduleWindow->width,
		(camera->fboID != 0) ? camera->textureHeight : Engine::GetInstance().moduleWindow->height);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glUseProgram(pickingShaderProgram);

	for (MeshRenderer* mesh : meshes)
	{
		if (!mesh || !mesh->GetEnabled()) continue;

		ResourceMesh* res = mesh->GetMeshResource();
		if (!res || !res->IsLoadedToMemory()) continue;

		if (!camera->GetFrustum()->InFrustum(mesh->GetGlobalAABB())) continue;

		UID id = mesh->owner->UUID;
		float r = ((id & 0x000000FF) >> 0) / 255.0f;
		float g = ((id & 0x0000FF00) >> 8) / 255.0f;
		float b = ((id & 0x00FF0000) >> 16) / 255.0f;
		float a = ((id & 0xFF000000) >> 24) / 255.0f;
		glUniform4f(pickingColorLoc, r, g, b, a);

		glm::mat4 model;
		mesh->owner->GetGlobalMatrix(model);
		glUniformMatrix4fv(pickingModelMatrixLoc, 1, GL_FALSE, glm::value_ptr(model));

		if (mesh->HasSkinning())
		{
			SkinnedMeshRenderer* skinnedMeshComp = (SkinnedMeshRenderer*)mesh;

			glUniformMatrix4fv(pickingMeshInverseLoc, 1, GL_FALSE, glm::value_ptr(skinnedMeshComp->GetMeshInverse()));

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, skinnedMeshComp->GetSSBOGlobal());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, skinnedMeshComp->GetSSBOOffset());

			glUniform1i(pickingHasBonesLoc, true);
		}
		else
		{
			glUniform1i(pickingHasBonesLoc, false);
		}

		glBindVertexArray(res->meshData.VAO);
		glDrawElements(GL_TRIANGLES, res->numIndices, GL_UNSIGNED_INT, 0);
	}

	UID pickedID = 0;
	unsigned char pixel[4];
	int targetHeight = (camera->fboID != 0) ? camera->textureHeight : Engine::GetInstance().moduleWindow->height;
	glReadPixels(x, targetHeight - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

	pickedID = pixel[0] + (pixel[1] << 8) + (pixel[2] << 16) + (pixel[3] << 24);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
	glEnable(GL_BLEND);

	return pickedID;
}

void ModuleRender::ChangeWindowSize(int x, int y)
{
	glViewport(0, 0, x, y);
}

void ModuleRender::OnEvent(const Event& event)
{
	switch (event.type)
	{
	case Event::Type::WindowResize:
	{
		{
			ChangeWindowSize(event.data.point.x, event.data.point.y);
		}
		break;
	}
	default:
		break;
	}
}