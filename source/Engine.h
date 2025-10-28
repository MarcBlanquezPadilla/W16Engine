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


class Engine
{
public:

	//Singelton
	static Engine& GetInstance();

	//Awake all modules
	bool Awake();

	//Start all modules
	bool Start();

	//Preupdate all modules
	bool PreUpdate();

	//Update all modules
	bool Update();

	//Postupdate all modules
	bool PostUpdate();

	//CleanUp all modules
	bool CleanUp();

	//Add modules to module list
	void AddModule(Module* module);

	float GetDtMs();
	float GetDtS();

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

private:

	Timer startTime;
	PerfTimer frameTime;

	float dt;

	std::list<Module*> moduleList;
};