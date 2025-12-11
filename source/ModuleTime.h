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
	bool CleanUp() override;

private:
	PerfTimer realTimeTimer;
	double lastTime;
};