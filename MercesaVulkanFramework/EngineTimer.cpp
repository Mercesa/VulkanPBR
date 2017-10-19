#include "EngineTimer.h"


void EngineTimer::Start()
{
	start = std::chrono::system_clock::now();
}

void EngineTimer::Update()
{
	std::chrono::duration<double> elapsed_seconds = start - end;

	end = start;
	start = std::chrono::system_clock::now();

	totalTime += elapsed_seconds.count();
	deltaTime = elapsed_seconds.count();
}

float EngineTimer::GetDeltaTime()
{
	return deltaTime;
}

float EngineTimer::GetTotalTime()
{
	return totalTime;
}