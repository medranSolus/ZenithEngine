#pragma once
#define _USE_MATH_DEFINES
#include "BasicTypes.h"
#include <cmath>
#include <random>

inline float wrap2Pi(float x)
{
	return static_cast<float>(fmod(x, 2.0f * M_PI));
}

inline float wrap(float x, float wrap)
{
	return static_cast<float>(fmod(x, wrap));
}

inline float randWrap2Pi(std::mt19937 & eng)
{
	return std::uniform_real_distribution<float>(0.0f, 2.0f * M_PI)(eng);
}

inline float randWrapNDC(std::mt19937 & eng)
{
	return std::uniform_real_distribution<float>(-1.0f, 1.0f)(eng);
}

inline float rand(float min, float max, std::mt19937 & eng)
{
	return std::uniform_real_distribution<float>(min, max)(eng);
}

inline float rand01(std::mt19937 & eng)
{
	return std::uniform_real_distribution<float>(0.0f, 1.0f)(eng);
}

inline GFX::Primitive::Color randColor(std::mt19937 & eng)
{
	return { rand01(eng), rand01(eng), rand01(eng) };
}
