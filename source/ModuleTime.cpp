#include "ModuleTime.h"
#include "ModuleEvents.h"
#include "Engine.h"
#include "utils/Timer.h"
#include "utils/Time.h"

float Time::time = 0.0f;
float Time::realTime = 0.0f;
float Time::deltaTime = 0.0f;
float Time::realDeltaTime = 0.0f;
float Time::timeScale = 0.0f;
int Time::frameCount = 0;

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
	
	isRunning = false;
	isPaused = false;
	oneFrameStep = false;

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
	if (isRunning)
	{
		if (oneFrameStep)
		{
			Time::deltaTime = 0.16f;
			oneFrameStep = false;
		}
		else
		{
			Time::deltaTime = Time::realDeltaTime * Time::timeScale;
		}
		Time::time += Time::deltaTime;
	}
	else
	{
		Time::deltaTime = 0.0f;
	}

	//FRAME COUNT
	Time::frameCount++;

	return ret;
}

void ModuleTime::Play()
{
    isRunning = true;
	isPaused = false;
    Time::timeScale = 1.0f;
	
	Time::time = 0.0f;
	Time::deltaTime = 0.0f;
	Engine::GetInstance().moduleEvents->PublishImmediate(Event::Type::Play);
}

void ModuleTime::Stop()
{
	isRunning = false;
	isPaused = false;
    Time::timeScale = 0.0f;

    Time::time = 0.0f;
    Time::deltaTime = 0.0f;
	Engine::GetInstance().moduleEvents->PublishImmediate(Event::Type::Stop);
}

void ModuleTime::Pause()
{
	if (!isRunning) return;

	isPaused = !isPaused;

	Time::timeScale = (isPaused) ? 0.0f : 1.0f;
	Engine::GetInstance().moduleEvents->PublishImmediate(Event(Event::Type::Pause, isPaused));
}

void ModuleTime::Step()
{
	if (isRunning && isPaused)
	{
		oneFrameStep = true;
	}
}

#pragma region Draw

