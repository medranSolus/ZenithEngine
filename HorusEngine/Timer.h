#pragma once
#include <chrono>

class Timer
{
	std::chrono::steady_clock::time_point lastMark;

public:
	inline Timer() noexcept : lastMark(std::chrono::steady_clock::now()) {}
	Timer(const Timer &) = default;
	Timer & operator=(const Timer &) = default;
	~Timer() = default;

	inline float Peek() const { return std::chrono::duration<float>(std::chrono::steady_clock::now() - lastMark).count(); }

	float Mark();
};