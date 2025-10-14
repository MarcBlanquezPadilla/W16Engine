#pragma once
#include <SDL3/SDL_timer.h>

class Timer
{
public:

	// Constructor
	Timer();

	void Start();
	float ReadSec() const;
	float ReadMSec() const;

private:
	int startTime;
};

class PerfTimer
{
public:

	// Constructor
	PerfTimer();

	void Start();
	double ReadMs() const;
	Uint64 ReadTicks() const;

private:
	Uint64 startTime;
	Uint64 frequency;
};