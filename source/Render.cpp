#include "Render.h"
#include "glad/glad.h"
#include <SDL3/sdl.h>
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
	
	float vertices[] = { -1,-1,0, 1, -1, 0, 0, 1, 0 };

	unsigned int my_id = 0;
	glGenBuffers(1, (GLuint*)&(my_id));
	glBindBuffer(GL_ARRAY_BUFFER, my_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	unsigned int my_indices = 0;

	int indices[] = { 0,1,2 };

	glGenBuffers(1, (GLuint*)&(my_indices));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, my_indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//CREAR SHADER
	unsigned int vShader = 0;
	const char* vertexShaderSource = "#version 460 core\n"
		"layout (location = 0) in vec3 position;\n"
		"void main()\n"
		"{\n"
		"gl_Position = vec4(position, 1.0f);\n"
		"}\n";
	
	if (!CreateShaderFromSources(vShader, GL_VERTEX_SHADER, vertexShaderSource, strlen(vertexShaderSource)))
		return false;

	//CREAR FRAGMENT
	unsigned int fShader = 0;
	const char* fragmentShaderSource = "#version 460 core\n"
		"out vec4 FragColor;\n"
		"void main()\n"
		"{\n"
		"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f); // Pintamos el triángulo de color naranja\n"
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
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
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
