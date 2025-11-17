#include "glad/glad.h"
#include <IL/il.h>
#include <glm/gtc/type_ptr.hpp>
#include <SDL3/sdl.h>

#include "Render.h"
#include "Window.h"
#include "Engine.h"
#include "Camera.h"
#include "utils/Frustum.h"
#include "Scene.h"
#include "GameObject.h"
#include "components/Mesh.h"
#include "components/Transform.h"
#include "components/Texture.h"
#include "utils/Log.h"

Render::Render(bool startEnabled) : Module(startEnabled)
{
	name = "Render";
}

Render::~Render()
{

}

bool Render::Awake()
{
	bool ret = true;

	int version = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress));

	if (version == 0)
	{
		LOG("Error loading the glad library");
		return false;
	}

	LOG("Initializing Devil");
	ilInit();
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
		
	LOG("Initializing Glad");
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glClearDepth(1.0f); 
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	gpu = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
	glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	glslVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

	//CREATE DEFAULT SHADER
	if (!CreateDefaultShader())
	{
		LOG("Error creating default shader");
		return false;
	}

	//CREATE NORMAL SHADER
	if (!CreateNormalShader())
	{
		LOG("Error creating normal shader");
		return false;
	}

	//CREATE STENCIL SHADER
	if (!CreateOutlineShader())
	{
		LOG("Error creating outline shader");
		return false;
	}
	
	//CREATE LINE SHADER
	if (!CreateLineShader())
	{
		LOG("Error creating outline shader");
		return false;
	}

	//CREATE CHECKER TEXTURE
	if (!CreateCheckerTexture())
	{
		LOG("Error creating checker texture");
		return false;
	}
	
	return ret;
}

bool Render::PreUpdate()
{
	bool ret = true;

	opaqueList.clear();
	transparentList.clear();
	linesList.clear();
	selectedMesh = nullptr;

	return ret;
}

bool Render::PostUpdate()
{
	bool ret = true;

	for (GameObject* gameObject : Engine::GetInstance().scene->GetGameObjects())
	{
		BuildRenderListsRecursive(gameObject);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearStencil(0);

	glUseProgram(shaderProgram);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	DrawRenderList(opaqueList);

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	DrawRenderList(transparentList);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	DrawLinesList(linesList);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	DrawStencil();

	glDisable(GL_STENCIL_TEST);
	glStencilMask(0xFF); 
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	return ret;
}

void Render::DrawRenderList(const std::multimap<float, RenderObject>& map)
{
	for (auto pair = map.rbegin(); pair != map.rend(); ++pair)
	{
		RenderObject renderObject = pair->second;

		//STENCIL
		if (renderObject.mesh->selected)
		{
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glStencilMask(0xFF);
		}
		else
		{
			glStencilFunc(GL_ALWAYS, 0, 0xFF);
			glStencilMask(0x00);
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderObject.textToBind);

		//DRAW MESH
		glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, glm::value_ptr(renderObject.globalModelMatrix));
		glUniform1i(hasUVsLoc, renderObject.mesh->hasUVs);

		glBindVertexArray(renderObject.mesh->meshData.VAO);
		glDrawElements(GL_TRIANGLES, renderObject.mesh->meshData.numIndices, GL_UNSIGNED_INT, 0);


		//DRAW NORMALS
		if (renderObject.mesh->drawNormals && renderObject.mesh->normalData.VAO != 0)
		{
			glUseProgram(normalShaderProgram);

			glUniformMatrix4fv(normalModelMatrixLoc, 1, GL_FALSE, glm::value_ptr(renderObject.globalModelMatrix));

			glBindVertexArray(renderObject.mesh->normalData.VAO);

			glDrawArrays(GL_LINES, 0, renderObject.mesh->normalData.numVertices);

			glUseProgram(shaderProgram);
		}
	}
}

void Render::DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color)
{
	RenderLine line = { start, end, color };
	linesList.push_back(line);
}

void Render::DrawLinesList(std::vector<RenderLine> list)
{
	for (RenderLine line : list)
	{
		glUseProgram(lineShaderProgram);

		glm::mat4 model = glm::mat4(1.0f);
		glUniformMatrix4fv(lineModelMatrixLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(lineViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(Engine::GetInstance().camera->GetViewMatrix()));
		glUniformMatrix4fv(lineProjectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(Engine::GetInstance().camera->GetProjectionMatrix()));

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

void Render::DrawStencil()
{
	GameObject* selectedGO = Engine::GetInstance().scene->GetSelectedGameObject();

	if (!selectedGO || !selectedGO->GetEnabled()) return;

	Mesh* selectedMesh = (Mesh*)selectedGO->GetComponent(ComponentType::Mesh);

	if (!selectedMesh || selectedMesh->stencilData.VAO == 0) return;

	Transform* selectedTransform = (Transform*)selectedGO->GetComponent(ComponentType::Transform);
	if (!selectedTransform) return;

	glUseProgram(outlineShaderProgram);

	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);

	glUniformMatrix4fv(outlineViewMatrixLoc, 1, GL_FALSE,glm::value_ptr(Engine::GetInstance().camera->GetViewMatrix()));
	glUniformMatrix4fv(outlineProjectionMatrixLoc, 1, GL_FALSE,glm::value_ptr(Engine::GetInstance().camera->GetProjectionMatrix()));
	glUniform4f(outlineColorLoc, 0.0f, 1.0f, 1.0f, 1.0f);
	glUniformMatrix4fv(outlineModelMatrixLoc, 1, GL_FALSE,glm::value_ptr(selectedTransform->GetGlobalMatrix()));

	glBindVertexArray(selectedMesh->stencilData.VAO);
	glDrawElements(GL_TRIANGLES, selectedMesh->stencilData.numVertices, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glUseProgram(0);
}

void Render::BuildRenderListsRecursive(GameObject* gameObject)
{
	if (gameObject && gameObject->enabled)
	{
		Transform* transform = (Transform*)gameObject->GetComponent(ComponentType::Transform);

		if (transform)
		{
			glm::mat4 globalModelMatrix = transform->GetGlobalMatrix();
			Mesh* mesh = (Mesh*)gameObject->GetComponent(ComponentType::Mesh);

			if (mesh && mesh->enabled && mesh->meshData.VAO != 0)
			{
				const AABB& globalAABB = mesh->aabb->GetGlobalAABB(transform->GetGlobalMatrix());

				if (Engine::GetInstance().camera->frustum->InFrustum(globalAABB))
				{
					Texture* texture = (Texture*)gameObject->GetComponent(ComponentType::Texture);
					unsigned int texToBind = checkerTextureID;

					if (texture)
					{
						if (texture->GetTextureID() != 0 && !texture->use_checker)
						{
							texToBind = texture->GetTextureID();
						}
					}

					RenderObject renderObject = { mesh, texToBind, globalModelMatrix };

					glm::vec3 aabbCenter = (globalAABB.min + globalAABB.max) * 0.5f;
					float distanceToCamera = glm::distance(aabbCenter, Engine::GetInstance().camera->GetPosition());

					if (texture && texture->transparent)
					{
						transparentList.emplace(distanceToCamera, renderObject);
					}
					else
					{
						opaqueList.emplace(distanceToCamera, renderObject);
					}
				}
			}
		}
		for (GameObject* go : gameObject->childs)
		{
			BuildRenderListsRecursive(go);
		}
	}

}



bool Render::CleanUp()
{
	bool ret = true;


	glDeleteProgram(shaderProgram);
	glDeleteProgram(normalShaderProgram);
	glDeleteProgram(outlineShaderProgram);
	ilShutDown();

	return ret;
}

bool Render::CreateShaderFromSources(unsigned int& shaderID, int type, const char* source, const int soruceLength)
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
			LOG("%s", logg);
			delete[] logg;
		}
		return false;
	}
	return true;
}

void Render::UpdateProjectionMatix(glm::mat4 pm)
{
	//UPDATE DEFAULT SHADER
	glUseProgram(shaderProgram);
	glUniformMatrix4fv(projectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(pm));

	//UPDATE NORMAL SHADER
	glUseProgram(normalShaderProgram);
	glUniformMatrix4fv(normalProjectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(pm));

	// UPDATE OUTLINER SHADER
	glUseProgram(outlineShaderProgram);
	glUniformMatrix4fv(outlineProjectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(pm));

	glUseProgram(lineShaderProgram);
	glUniformMatrix4fv(lineProjectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(pm));

	glUseProgram(shaderProgram);
}

void Render::UpdateViewMatix(glm::mat4 vm)
{
	//UPDATE DEFAULT SHADER
	glUseProgram(shaderProgram);
	glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, glm::value_ptr(vm));

	//UPDATE NORMAL SHADER
	glUseProgram(normalShaderProgram);
	glUniformMatrix4fv(normalViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(vm));

	// UPDATE OUTLINER SHADER
	glUseProgram(outlineShaderProgram);
	glUniformMatrix4fv(outlineViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(vm));

	glUseProgram(lineShaderProgram);
	glUniformMatrix4fv(lineViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(vm));

	glUseProgram(shaderProgram);
}

bool Render::CreateDefaultShader()
{
	unsigned int vShader = 0;
	const char* vertexShaderSource = "#version 460 core\n"
		"layout (location = 0) in vec3 position;\n"
		"layout (location = 1) in vec2 aTexCoord;\n"
		"uniform mat4 model; \n"
		"uniform mat4 view; \n"
		"uniform mat4 projection; \n"
		"out vec3 localPos; \n"     
		"out vec2 texCoord; \n"     
		"void main()\n"
		"{\n"
		"   gl_Position = projection * view * model * vec4(position, 1.0f);\n"
		"   localPos = position;\n"
		"   texCoord = aTexCoord;\n"
		"}\n";

	if (!CreateShaderFromSources(vShader, GL_VERTEX_SHADER, vertexShaderSource, strlen(vertexShaderSource)))
		return false;

	unsigned int fShader = 0;
	const char* fragmentShaderSource = "#version 460 core\n"
		"in vec3 localPos;\n"
		"in vec2 texCoord;\n"
		"out vec4 color;\n"
		"uniform sampler2D texture1;\n"
		"uniform bool u_hasUVs;\n"
		"void main()\n"
		"{\n"
		"   vec2 uv = texCoord;\n"
		"   if (!u_hasUVs)\n"  
		"   {\n"
		"       uv = localPos.xz * 0.5; \n"
		"   }\n"
		"   color = texture(texture1, uv);\n"
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
			LOG("%s", logg);
			delete[] logg;
		}
		return false;
	}
	glDeleteShader(vShader);
	glDeleteShader(fShader);

	modelMatrixLoc = glGetUniformLocation(shaderProgram, "model");
	viewMatrixLoc = glGetUniformLocation(shaderProgram, "view");
	projectionMatrixLoc = glGetUniformLocation(shaderProgram, "projection");
	hasUVsLoc = glGetUniformLocation(shaderProgram, "u_hasUVs");

	return true;
}

bool Render::CreateNormalShader()
{
	unsigned int vShader = 0;
	const char* vertexShaderSource = "#version 460 core\n"
		"layout (location = 0) in vec3 position;\n"
		"uniform mat4 model; \n"
		"uniform mat4 view; \n"
		"uniform mat4 projection; \n"
		"void main()\n"
		"{\n"
		"   gl_Position = projection * view * model * vec4(position, 1.0f);\n"
		"}\n";

	if (!CreateShaderFromSources(vShader, GL_VERTEX_SHADER, vertexShaderSource, strlen(vertexShaderSource)))
		return false;

	unsigned int fShader = 0;
	const char* fragmentShaderSource = "#version 460 core\n"
		"out vec4 color;\n"
		"void main() { color = vec4(1.0, 1.0, 0.0, 1.0); }\n";

	if (!CreateShaderFromSources(fShader, GL_FRAGMENT_SHADER, fragmentShaderSource, strlen(fragmentShaderSource)))
		return false;

	normalShaderProgram = glCreateProgram();
	glAttachShader(normalShaderProgram, vShader);
	glAttachShader(normalShaderProgram, fShader);
	glLinkProgram(normalShaderProgram);

	int status = 0;
	glGetProgramiv(normalShaderProgram, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		LOG("Error linking normal shader!");
		return false;
	}

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	normalModelMatrixLoc = glGetUniformLocation(normalShaderProgram, "model");
	normalViewMatrixLoc = glGetUniformLocation(normalShaderProgram, "view");
	normalProjectionMatrixLoc = glGetUniformLocation(normalShaderProgram, "projection");

	return true;
}

bool Render::CreateOutlineShader()
{
	unsigned int vShader = 0;
	const char* vertexShaderSource = "#version 460 core\n"
		"layout (location = 0) in vec3 position;\n"
		"layout (location = 2) in vec3 aNormal;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"uniform float u_outlineThickness = 0.03;\n"
		"void main()\n"
		"{\n"
		// Extraer la escala
		"   vec3 scale = vec3(\n"
		"       length(model[0].xyz),\n"
		"       length(model[1].xyz),\n"
		"       length(model[2].xyz)\n"
		"   );\n"
		// Crear matriz sin escala
		"   mat4 modelNoScale = model;\n"
		"   modelNoScale[0].xyz /= scale.x;\n"
		"   modelNoScale[1].xyz /= scale.y;\n"
		"   modelNoScale[2].xyz /= scale.z;\n"
		// Transformar normal sin escala
		"   vec3 worldNormal = normalize(mat3(modelNoScale) * aNormal);\n"
		// Calcular posición base
		"   vec4 worldPos = model * vec4(position, 1.0);\n"
		// Calcular distancia para outline dinámico
		"   vec4 viewPos = view * worldPos;\n"
		"   float distance = length(viewPos.xyz);\n"
		"   float dynamicThickness = u_outlineThickness * (distance * 0.1);\n"
		// Desplazar en espacio mundo
		"   worldPos.xyz += worldNormal * dynamicThickness;\n"
		"   gl_Position = projection * view * worldPos;\n"
		"}\n";

	if (!CreateShaderFromSources(vShader, GL_VERTEX_SHADER, vertexShaderSource, strlen(vertexShaderSource)))
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
		LOG("Error linking outline shader!");
		return false;
	}

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	outlineModelMatrixLoc = glGetUniformLocation(outlineShaderProgram, "model");
	outlineViewMatrixLoc = glGetUniformLocation(outlineShaderProgram, "view");
	outlineProjectionMatrixLoc = glGetUniformLocation(outlineShaderProgram, "projection");
	outlineColorLoc = glGetUniformLocation(outlineShaderProgram, "outlineColor");

	return true;
}

bool Render::CreateLineShader()
{
	const char* vsSource = "#version 460 core\n"
		"layout (location = 0) in vec3 position;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
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
	lineViewMatrixLoc = glGetUniformLocation(lineShaderProgram, "view");
	lineProjectionMatrixLoc = glGetUniformLocation(lineShaderProgram, "projection");
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

bool Render::CreateCheckerTexture()
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

bool Render::UploadMeshToGPU(MeshData& meshData, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)

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


	glBindVertexArray(0);

	meshData.numIndices = indices.size();

	LOG("Mesh uploaded to GPU. VAO: %u, VBO: %u, EBO: %u, Indices: %d",
		meshData.VAO, meshData.VBO, meshData.EBO, meshData.numIndices);

	return true;

}

bool Render::UploadSmoothedMeshToGPU(unsigned int& vao, unsigned int& vbo, unsigned int& sharedEbo ,const std::vector<Vertex>& vertices)
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sharedEbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	LOG("Outline smoothed mesh upload to GPU. VAO: %u, VBO: % u", vao, vbo);
	return true;
}

bool Render::UploadLinesToGPU(unsigned int& vao, unsigned int& vbo, const std::vector<glm::vec3>& lines)
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

	LOG("Lines of normals uploaded to GPU. VAO: %u, Vertices: %d", vao, lines.size());
	return true;
}

void Render::DeleteMeshFromGPU(MeshData& meshData)
{
	LOG("Mesh removed from GPU. VAO: %d, EBO: %d, VBO: %d", meshData.VAO, meshData.EBO, meshData.VBO);
	if (meshData.VBO != 0) glDeleteBuffers(1, &meshData.VBO);
	if (meshData.EBO != 0) glDeleteBuffers(1, &meshData.EBO);
	if (meshData.VAO != 0) glDeleteVertexArrays(1, &meshData.VAO);
	meshData = MeshData();
}

unsigned int Render::UploadTextureToGPU(unsigned char* data, int width, int height)
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

	LOG("Texture uploaded to GPU. ID: %u", textureID);
	return textureID;
}

void Render::DeleteTextureFromGPU(unsigned int textureID)
{
	if (textureID != 0)
	{
		glDeleteTextures(1, &textureID);
		LOG("Texture removed from GPU. ID: %u", textureID);
	}
}

void Render::ChangeWindowSize(int x, int y)
{
	glViewport(0, 0, x, y);
}