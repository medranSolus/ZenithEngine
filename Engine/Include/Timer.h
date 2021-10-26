#pragma once
#include <chrono>

namespace ZE
{
	// Simple timer for time measurement
	class Timer final
	{
		std::chrono::steady_clock::time_point lastMark;

	public:
		Timer() noexcept : lastMark(std::chrono::steady_clock::now()) {}
		Timer(Timer&&) = default;
		Timer(const Timer&) = default;
		Timer& operator=(Timer&&) = default;
		Timer& operator=(const Timer&) = default;
		~Timer() = default;

		float Peek() const noexcept { return std::chrono::duration<float>(std::chrono::steady_clock::now() - lastMark).count(); }

		float Mark() noexcept;
	};
}