#pragma once
#include "Module.h"
#include "utils/Timer.h"

class ModuleTime : public Module
{
public:

	ModuleTime(bool startEnabled);

	virtual ~ModuleTime();

	bool Awake() override;

	bool PreUpdate() override;

	void Play();
	void Stop();
	void Pause();
	void Step();

	const bool GetIsRunning() { return isRunning; };
	const bool GetIsPaused() { return isPaused; };

private:
	PerfTimer realTimeTimer;
	double lastTime;

	bool isRunning;
	bool isPaused;
	bool oneFrameStep;
};