#pragma once
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

inline float randWrap2Pi()
{
	return (rand() % 628315) / 100000.0f;
}

inline float randWrapNDC()
{
	return (rand() % 2000000 - 1000000) / 1000000.0f;
}

inline float rand(float min, float max, std::mt19937 & eng)
{
	return std::uniform_real_distribution<float>(min, max)(eng);
}

inline float rand01()
{
	return (rand() % 1000) / 1000.0f;
}

inline GFX::Primitive::Color randColor()
{
	return { rand01(), rand01(), rand01() };
}
