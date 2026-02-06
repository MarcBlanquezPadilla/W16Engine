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
class ModuleTime;
class ModuleWindow;
class ModuleInput;
class ModuleRender;
class ModuleScene;
class ModuleEditor;
class ModuleLoader;
class ModuleEvents;
class ModuleResources;
class ModulePhysics;


class Engine
{

public:

	static Engine& GetInstance();

	bool Awake();

	bool Start();

	bool PreUpdate();

	bool Update();
	
	bool FixedUpdate();

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
	
	ModuleTime* moduleTime;
	ModuleWindow* moduleWindow;
	ModuleInput* moduleInput;
	ModulePhysics* modulePhysics;
	ModuleRender* moduleRender;
	ModuleScene* moduleScene;
	ModuleEditor* moduleEditor;
	ModuleLoader* moduleLoader;
	ModuleEvents* moduleEvents;
	ModuleResources* moduleResources;

private:

	bool quit;

	std::vector<Module*> moduleList;
};