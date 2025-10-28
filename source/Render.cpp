#include "Render.h"
#include "glad/glad.h"
#include <SDL3/sdl.h>
#include "Log.h"
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


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

	for (const Mesh& mesh : loadedMeshes)
	{
		glBindVertexArray(mesh.VAO);
		glDrawElements(GL_TRIANGLES, mesh.numIndices, GL_UNSIGNED_INT, 0);
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
	// --- Este es el Vertex Shader SIN UVs ---
	const char* vertexShaderSource = "#version 460 core\n"
		"layout (location = 0) in vec3 position;\n" // Solo recibe posición
		"uniform mat4 model; \n"
		"uniform mat4 view; \n"
		"uniform mat4 projection; \n"
		"out vec3 worldPos; \n" // Pasamos la posición en el mundo
		"void main()\n"
		"{\n"
		"   vec4 posInWorld = model * vec4(position, 1.0f);\n"
		"   gl_Position = projection * view * posInWorld;\n"
		"   worldPos = posInWorld.xyz;\n" // Pasamos la posición al Fragment
		"}\n";

	if (!CreateShaderFromSources(vShader, GL_VERTEX_SHADER, vertexShaderSource, strlen(vertexShaderSource)))
		return false;

	unsigned int fShader = 0;
	// --- Este es el Fragment Shader SIN UVs ---
	const char* fragmentShaderSource = "#version 460 core\n"
		"in vec3 worldPos;\n" // Recibe la posición del mundo
		"out vec4 color;\n"
		"uniform sampler2D texture1;\n"
		"void main()\n"
		"{\n"
		// "Fingimos" las UVs usando las coordenadas X y Z del mundo
		"   vec2 fakeUVs = worldPos.xz * 0.5; \n"
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

	// --- CARGAR TEXTURAS (copiado de tu Awake) ---
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &checkerTextureID); // Asumo que 'textureID' es una variable miembro de Render.h
	glBindTexture(GL_TEXTURE_2D, checkerTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// ¡OJO! El orden de tu código original estaba mal
	// glGenerateMipmap va DESPUÉS de glTexImage2D
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CHECKERS_WIDTH, CHECKERS_HEIGHT,
		0, GL_RGBA, GL_UNSIGNED_BYTE, checkerImage);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0); // Desvincular por seguridad

	return true; // Todo ha ido bien
}

bool Render::LoadModel(const std::string& filePath)
{
	// 1. Limpiamos las mallas anteriores
	// El destructor de la struct 'Mesh' se llamará y limpiará la VRAM
	loadedMeshes.clear();
	LOG("Limpiando mallas anteriores. Cargando nuevo modelo: %s", filePath.c_str());

	// 2. Usar Assimp para leer el archivo
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath,
		aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG("Error cargando modelo con Assimp: %s", importer.GetErrorString());
		return false;
	}

	// 3. Procesar todas las mallas en la escena
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* currentMesh = scene->mMeshes[i];
		ProcessAndUploadMesh(currentMesh); // Llamamos a nuestra función privada
	}

	LOG("¡Modelo cargado con éxito! %d mallas subidas a GPU.", loadedMeshes.size());
	return true;
}

void Render::ProcessAndUploadMesh(aiMesh* assimpMesh)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	// 1. Extraer Vértices (Solo Posición)
	for (unsigned int i = 0; i < assimpMesh->mNumVertices; i++)
	{
		Vertex vertex;
		vertex.position.x = assimpMesh->mVertices[i].x;
		vertex.position.y = assimpMesh->mVertices[i].y;
		vertex.position.z = assimpMesh->mVertices[i].z;
		vertices.push_back(vertex);
	}

	// 2. Extraer Índices
	for (unsigned int i = 0; i < assimpMesh->mNumFaces; i++) {
		aiFace face = assimpMesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	if (vertices.empty() || indices.empty()) {
		LOG("Error: Malla de Assimp vacía, saltando.");
		return;
	}

	// 3. Crear la nueva malla en nuestra lista
	loadedMeshes.emplace_back();
	Mesh& newMesh = loadedMeshes.back(); // Obtenemos una referencia a ella

	// 4. Subir datos a la GPU (VAO, VBO, EBO)
	glGenVertexArrays(1, &newMesh.VAO);
	glBindVertexArray(newMesh.VAO);

	glGenBuffers(1, &newMesh.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, newMesh.VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &newMesh.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newMesh.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// 5. Configurar Atributos (Solo Posición)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	glBindVertexArray(0);

	// 6. Guardar el contador de índices
	newMesh.numIndices = indices.size();
}
