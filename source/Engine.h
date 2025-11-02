#pragma once
#include "Timer.h"
#include <memory>
#include <list>

enum EngineState
{
	CREATE = 1,
	AWAKE,
	START,
	LOOP,
	CLEAN,
	FAIL,
	EXIT
};

class Module;
class Window;
class OpenGL;
class Input;
class Render;
class Camera;
class Scene;
class Editor;
class Loader;


class Engine
{
public:

	static Engine& GetInstance();

	bool Awake();

	bool Start();

	bool PreUpdate();

	bool Update();

	bool PostUpdate();

	bool CleanUp();

	void AddModule(Module* module);

	float GetDtMs();
	float GetDtS();

	void QuitApplication();

private:

	Engine();
	~Engine() {};

public: 
	
	Window* window;
	Input* input;
	OpenGL* openGL;
	Render* render;
	Camera* camera;
	Scene* scene;
	Editor* editor;
	Loader* loader;

private:

	Timer startTime;
	PerfTimer frameTime;

	float dt;
	bool quit;

	std::list<Module*> moduleList;
};