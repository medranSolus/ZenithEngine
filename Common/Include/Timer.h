#pragma once
#include "Macros.h"
#include <chrono>

namespace ZE
{
	// Simple timer for time measurement
	class Timer final
	{
		std::chrono::steady_clock::time_point lastMark;

	public:
		Timer() noexcept : lastMark(std::chrono::steady_clock::now()) {}
		ZE_CLASS_DEFAULT(Timer);
		~Timer() = default;

		float Peek() const noexcept { return std::chrono::duration<float>(std::chrono::steady_clock::now() - lastMark).count(); }

		float Mark() noexcept;
	};
}