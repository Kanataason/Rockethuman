#pragma once
#include <chrono>
class Timer
{
public :
	void Initialize();

	float Tick();

private:
	std::chrono::high_resolution_clock::time_point _prevTime;
};