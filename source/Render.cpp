#include "Render.h"
#include "glad/glad.h"
#include <SDL3/sdl.h>
#include "Log.h"
#include <glm/gtc/type_ptr.hpp>


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
	glClearDepth(1.0f); glClearColor(0.f, 0.f, 0.f, 1.f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	
	float vertices[] = {
		// Formato: X,   Y,  Z,    U,   V

		// Cara Frontal (Triángulo 1)
		-1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
		0.0f,  1.0f,  0.0f, 0.5f, 1.0f,

		1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
		0.0f, -1.0f, -1.0f, 1.0f, 0.0f,
		0.0f,  1.0f,  0.0f, 0.5f, 1.0f,

		0.0f, -1.0f, -1.0f, 0.0f, 0.0f,
		-1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
		0.0f,  1.0f,  0.0f, 0.5f, 1.0f,

		-1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
		0.0f, -1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.5f, 1.0f
	};

	unsigned int my_id = 0;
	glGenBuffers(1, (GLuint*)&(my_id));
	glBindBuffer(GL_ARRAY_BUFFER, my_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);


	//CREAR SHADER
	unsigned int vShader = 0;
	const char* vertexShaderSource = "#version 460 core\n"
		"layout (location = 0) in vec3 position;\n"  
		"layout (location = 1) in vec2 aTexCoord;\n" 

		"uniform mat4 model; \n"       
		"uniform mat4 view; \n"
		"uniform mat4 projection; \n"  

		"out vec2 texCoord; \n"

		"void main()\n"
		"{\n"
		"    // Transformación MVP: Proyección * Vista * Modelo * Posición\n"
		"    gl_Position = projection * view  * vec4(position, 1.0f);\n"
		"    texCoord = aTexCoord;\n"
		"}\n";
	
	if (!CreateShaderFromSources(vShader, GL_VERTEX_SHADER, vertexShaderSource, strlen(vertexShaderSource)))
		return false;

	//CREAR FRAGMENT
	unsigned int fShader = 0;
	const char* fragmentShaderSource = "#version 460 core\n"
		"in vec2 texCoord;\n" // <-- Debe ser vec2 (recibido del Vertex Shader)
		"out vec4 color;\n"
		"layout(location = 0) uniform sampler2D texture1;\n" // <-- Corregido el nombre a 'texture1'
		"void main()\n"
		"{\n"
		"color = texture(texture1, texCoord);\n" // <-- Ahora usa vec2
		"}\n";

	if (!CreateShaderFromSources(fShader, GL_FRAGMENT_SHADER, fragmentShaderSource, strlen(fragmentShaderSource) - 1))
		return false;


	//CREAR PROGRAM
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

	//CREATE TEXTURE CHECKERS
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

	//CARGAR TEXTURAS
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CHECKERS_WIDTH, CHECKERS_HEIGHT,
		0, GL_RGBA, GL_UNSIGNED_BYTE, checkerImage);
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
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
	//glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
	glDrawArrays(GL_TRIANGLES, 0, 12);
	glBindVertexArray(0);
	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);

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
