#include "Timer.h"

using namespace std::chrono;

float fpsSec = 0;
void Timer::Update()
{
	time_point<steady_clock> curTime = steady_clock::now();
	spf = duration<double, std::deca>(curTime - prev).count();
	prev = curTime;

	fpsSec += spf;

	if (fpsSec < 0.5f) // update fps per 0.5 sec
		return;
	fpsSec = 0;

	fps = 1.0f / spf;
}

float Timer::Elapsed()
{
	return duration<double, std::deca>(steady_clock::now() - first).count();
}

Timer::Timer()
{

	fps = 0;
	spf = 0;
	prev = steady_clock::now();
	first = steady_clock::now();
}

