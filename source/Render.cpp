#include "glad/glad.h"
#include <IL/il.h>
#include <glm/gtc/type_ptr.hpp>
#include <SDL3/sdl.h>

#include "Render.h"
#include "Window.h"
#include "Engine.h"
#include "Scene.h"
#include "GameObject.h"
#include "components/Mesh.h"
#include "components/Transform.h"
#include "components/Texture.h"
#include "Log.h"

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

	return ret;
}

bool Render::PostUpdate()
{
	bool ret = true;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgram);

	const std::vector<GameObject*>& gameObjects = Engine::GetInstance().scene->GetGameObjects();

	for (GameObject* go : gameObjects)
	{
		RecursiveGameObjectsDraw(go);
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	return ret;
}

bool Render::RecursiveGameObjectsDraw(GameObject* gameObject)
{
	unsigned int defaultTexture = checkerTextureID;

	Mesh* mesh = (Mesh*)gameObject->GetComponent(ComponentType::Mesh);
	Transform* transform = (Transform*)gameObject->GetComponent(ComponentType::Transform);
	Texture* texture = (Texture*)gameObject->GetComponent(ComponentType::Texture);

	if (gameObject->enabled)
	{
		glm::mat4 globalModelMatrix = transform->GetGlobalMatrix();
		if (mesh && mesh->enabled && transform && mesh->meshData.VAO != 0)
		{
			unsigned int texToBind = defaultTexture;

			if (texture != nullptr)
			{
				if (texture->use_checker)
				{
					texToBind = checkerTextureID;
				}
				else if (texture->GetTextureID() != 0)
				{
					texToBind = texture->GetTextureID();
				}
			}

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texToBind);

			//DRAW MESH
			glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, glm::value_ptr(globalModelMatrix));
			glUniform1i(hasUVsLoc, mesh->hasUVs);

			glBindVertexArray(mesh->meshData.VAO);
			glDrawElements(GL_TRIANGLES, mesh->meshData.numIndices, GL_UNSIGNED_INT, 0);


			//DRAW NORMALS
			if (mesh->drawNormals && mesh->normalData.VAO != 0)
			{
				glUseProgram(normalShaderProgram);

				glUniformMatrix4fv(normalModelMatrixLoc, 1, GL_FALSE, glm::value_ptr(globalModelMatrix));

				glBindVertexArray(mesh->normalData.VAO);

				glDrawArrays(GL_LINES, 0, mesh->normalData.numVertices);

				glUseProgram(shaderProgram);
			}
		}

		for (GameObject* go : gameObject->childs)
		{
			RecursiveGameObjectsDraw(go);
		}
	}

	return true;
}

bool Render::CleanUp()
{
	bool ret = true;

	ilShutDown();
	glDeleteProgram(shaderProgram);

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

	glBindVertexArray(0);

	meshData.numIndices = indices.size();

	LOG("Mesh uploaded to GPU. VAO: %u, VBO: %u, EBO: %u, Indices: %d",
		meshData.VAO, meshData.VBO, meshData.EBO, meshData.numIndices);

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