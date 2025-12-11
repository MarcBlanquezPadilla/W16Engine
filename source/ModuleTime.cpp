#include "ModuleTime.h"
#include "utils/Timer.h"
#include "utils/Time.h"


ModuleTime::ModuleTime(bool startEnabled) : Module(startEnabled)
{
	name = "Time";
}

ModuleTime::~ModuleTime()
{

}

bool ModuleTime::Awake()
{
	bool ret = true;

	realTimeTimer.Start();
	lastTime = 0;

	return ret;
}

bool ModuleTime::PreUpdate()
{
	bool ret = true;

	//APPLICATION TIMERS
	Time::realTime = realTimeTimer.ReadSec();
	Time::realDeltaTime = Time::realTime - lastTime;
	lastTime = Time::realTime;

	//GAME TIMERS
	Time::deltaTime = Time::realDeltaTime * Time::timeScale;
	Time::time += Time::deltaTime;
	
	Time::frameCount++;


	return ret;
}


bool ModuleTime::CleanUp()
{
	bool ret = true;

	

	return ret;
}

#pragma region Draw

