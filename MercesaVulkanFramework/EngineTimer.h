#pragma once

#include <chrono>
#include <ctime>

class EngineTimer
{
public:
	EngineTimer() = default;
	~EngineTimer() = default;

	void Start();
	void Update();
	float GetDeltaTime();
	float GetTotalTime();

private:
	std::chrono::time_point<std::chrono::system_clock> start, end;

	double totalTime = 0.0f;
	double deltaTime = 0.0f;
};

