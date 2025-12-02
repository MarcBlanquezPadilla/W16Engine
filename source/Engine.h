#pragma once
#include "utils/Timer.h"
#include <memory>
#include <vector>

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
class ModuleWindow;
class ModuleInput;
class ModuleRender;
class ModuleScene;
class ModuleEditor;
class ModuleLoader;
class ModuleEvents;


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
	
	ModuleWindow* moduleWindow;
	ModuleInput* moduleInput;
	ModuleRender* moduleRender;
	ModuleScene* moduleScene;
	ModuleEditor* moduleEditor;
	ModuleLoader* moduleLoader;
	ModuleEvents* moduleEvents;


private:

	Timer startTime;
	PerfTimer frameTime;

	float dt;
	bool quit;

	std::vector<Module*> moduleList;
};