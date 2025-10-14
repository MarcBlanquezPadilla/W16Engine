#pragma once
#include "Timer.h"

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

class Engine
{
public:

	static Engine& GetInstance();

	bool Awake();

	bool Start();

	bool Update();

	bool CleanUp();

private:

	Engine();
	~Engine() {};

	Timer startTime;
	PerfTimer frameTime;

	float dt;
};