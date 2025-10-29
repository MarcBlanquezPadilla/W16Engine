#include "Render.h"
#include "Engine.h"
#include "GameObject.h"
#include "Scene.h"
#include "glad/glad.h"
#include <SDL3/sdl.h>
#include "Log.h"
#include <glm/gtc/type_ptr.hpp>
#include "components/Mesh.h"
#include "components/Transform.h"


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
		LOG("Error loading the glad library");

	LOG("Vendor: %s", glGetString(GL_VENDOR));
	LOG("Renderer: %s", glGetString(GL_RENDERER));
	LOG("OpenGL version supported %s", glGetString(GL_VERSION));
	LOG("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));


	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glClearDepth(1.0f); 
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	if (!CreateDefaultShader())
	{
		LOG("Error creating default shader");
		return false;
	}

	// 4. Crear la Textura de Damero por defecto
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

	glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, checkerTextureID);

	const std::vector<GameObject*>& gameObjects = Engine::GetInstance().scene->GetGameObjects();

	for (GameObject* go : gameObjects)
	{
		Mesh* mesh = (Mesh*)go->GetComponent(ComponentType::Mesh);
		Transform* transform = (Transform*)go->GetComponent(ComponentType::Transform);

		if (mesh && transform && mesh->meshData.VAO != 0)
		{
			glm::mat4 modelMatrix = transform->GetLocalMatrix();
			glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			glBindVertexArray(mesh->meshData.VAO);
			glDrawElements(GL_TRIANGLES, mesh->meshData.numIndices, GL_UNSIGNED_INT, 0);
		}
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	return ret;
}

bool Render::CleanUp()
{
	bool ret = true;

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
	glUseProgram(shaderProgram);

	GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pm));
}

void Render::UpdateViewMatix(glm::mat4 vm)
{
	glUseProgram(shaderProgram);

	GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(vm));
}

bool Render::CreateDefaultShader()
{
	unsigned int vShader = 0;
	const char* vertexShaderSource = "#version 460 core\n"
		"layout (location = 0) in vec3 position;\n"
		"uniform mat4 model; \n"
		"uniform mat4 view; \n"
		"uniform mat4 projection; \n"
		"out vec3 localPos; \n"
		"void main()\n"
		"{\n"
		"   vec4 posInWorld = model * vec4(position, 1.0f);\n"
		"   gl_Position = projection * view * posInWorld;\n"
		"   localPos = position;\n"
		"}\n";

	if (!CreateShaderFromSources(vShader, GL_VERTEX_SHADER, vertexShaderSource, strlen(vertexShaderSource)))
		return false;

	unsigned int fShader = 0;
	// --- Este es el Fragment Shader SIN UVs ---
	const char* fragmentShaderSource = "#version 460 core\n"
		"in vec3 localPos;\n" // Recibe la posición del mundo
		"out vec4 color;\n"
		"uniform sampler2D texture1;\n"
		"void main()\n"
		"{\n"
		// "Fingimos" las UVs usando las coordenadas X y Z del mundo
		"   vec2 fakeUVs = localPos.xz * 0.5; \n"
		"   color = texture(texture1, fakeUVs);\n"
		"}\n";

	if (!CreateShaderFromSources(fShader, GL_FRAGMENT_SHADER, fragmentShaderSource, strlen(fragmentShaderSource)))
		return false;

	// --- CREAR PROGRAM (copiado de tu Awake) ---
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
		return false; // Falla aquí si el linkeo va mal
	}
	glDeleteShader(vShader);
	glDeleteShader(fShader);

	// --- ¡EXTRA! Cachear los uniforms ---
	// (Esto lo teníamos en el Render.h de antes)
	modelMatrixLoc = glGetUniformLocation(shaderProgram, "model");
	viewMatrixLoc = glGetUniformLocation(shaderProgram, "view");
	projectionMatrixLoc = glGetUniformLocation(shaderProgram, "projection");

	return true; // Todo ha ido bien
}

bool Render::CreateCheckerTexture()
{
	// --- CREATE TEXTURE CHECKERS (copiado de tu Awake) ---
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
	// 1. Crear VAO (Vertex Array Object)
	glGenVertexArrays(1, &meshData.VAO);
	glBindVertexArray(meshData.VAO);

	// 2. Crear y llenar VBO (Vertex Buffer Object)
	// Esto crea el buffer de vértices y guarda su ID en meshData.VBO
	glGenBuffers(1, &meshData.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, meshData.VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	// 3. Crear y llenar EBO (Element Buffer Object / Index Buffer)
	// Esto crea el buffer de índices y guarda su ID en meshData.EBO
	glGenBuffers(1, &meshData.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// 4. Configurar Atributos (Solo Posición)
	// Le dice al VAO cómo leer el VBO
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	// 5. Desvincular VAO (buena práctica)
	glBindVertexArray(0);

	// 6. Guardar el número de índices
	meshData.numIndices = indices.size();

	LOG("Malla subida a GPU. VAO: %u, VBO: %u, EBO: %u, Índices: %d",
		meshData.VAO, meshData.VBO, meshData.EBO, meshData.numIndices);

	return true;
}

void Render::DeleteMeshFromGPU(MeshData& meshData)
{
	if (meshData.VBO != 0) glDeleteBuffers(1, &meshData.VBO);
	if (meshData.EBO != 0) glDeleteBuffers(1, &meshData.EBO);
	if (meshData.VAO != 0) glDeleteVertexArrays(1, &meshData.VAO);
	meshData = MeshData(); // Resetea la struct
}
