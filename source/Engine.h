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
class Input;

class Engine
{
public:

	//Singelton
	static Engine& GetInstance();

	//Awake all modules
	bool Awake();

	//Start all modules
	bool Start();

	//Update all modules
	bool Update();

	//CleanUp all modules
	bool CleanUp();

	//Add modules to module list
	void AddModule(Module* module);

private:

	Engine();
	~Engine() {};

public: 
	
	Window* window;
	Input* input;

private:

	Timer startTime;
	PerfTimer frameTime;

	float dt;

	std::list<Module*> moduleList;
};