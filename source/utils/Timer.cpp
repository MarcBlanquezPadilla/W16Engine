#include "Timer.h"

Timer::Timer()
{
	Start();
}

void Timer::Start()
{
	startTime = SDL_GetTicks();
}

float Timer::ReadSec() const
{
	return (float)(SDL_GetTicks() - startTime) / 1000.0f;
}

float Timer::ReadMSec() const
{
	return (float)(SDL_GetTicks() - startTime);
}

PerfTimer::PerfTimer()
{
	Start();
}

void PerfTimer::Start()
{
	frequency = SDL_GetPerformanceFrequency();
	startTime = SDL_GetPerformanceCounter();
}

double PerfTimer::ReadMs() const
{
	return ((double)(SDL_GetPerformanceCounter() - startTime) / frequency * 1000);
}

Uint64 PerfTimer::ReadTicks() const
{
	return SDL_GetPerformanceCounter() - startTime;
}