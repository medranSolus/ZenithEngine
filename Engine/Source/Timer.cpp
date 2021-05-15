#include "Timer.h"

namespace ZE
{
	float Timer::Mark() noexcept
	{
		std::chrono::steady_clock::time_point before = lastMark;
		lastMark = std::chrono::steady_clock::now();
		return std::chrono::duration<float>(lastMark - before).count();
	}
}