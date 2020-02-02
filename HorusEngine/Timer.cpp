#include "Timer.h"

float Timer::Mark()
{
	std::chrono::steady_clock::time_point before = lastMark;
	lastMark = std::chrono::steady_clock::now();
	return std::chrono::duration<float>(lastMark - before).count();
}