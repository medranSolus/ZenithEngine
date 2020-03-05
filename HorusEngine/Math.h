#pragma once
#define _USE_MATH_DEFINES
#include "Vertex.h"
#include <cmath>
#include <random>

constexpr bool equals(float a, float b)
{
	return a >= b - FLT_EPSILON && a <= b + FLT_EPSILON;
}

constexpr bool notEquals(float a, float b)
{
	return a <= b - FLT_EPSILON || a >= b + FLT_EPSILON;
}

inline float wrap2Pi(float x)
{
	return static_cast<float>(fmod(x, 2.0f * M_PI));
}

inline float wrap(float x, float wrap)
{
	return static_cast<float>(fmod(x, wrap));
}

inline float randWrap2Pi(std::mt19937_64& eng)
{
	return std::uniform_real_distribution<float>(0.0f, 2.0f * M_PI)(eng);
}

inline float randWrapNDC(std::mt19937_64& eng)
{
	return std::uniform_real_distribution<float>(-1.0f, 1.0f)(eng);
}

inline float rand(float min, float max, std::mt19937_64& eng)
{
	return std::uniform_real_distribution<float>(min, max)(eng);
}

inline DirectX::XMFLOAT3 randPosition(float min, float max, std::mt19937_64& eng)
{
	return { rand(min, max, eng), rand(min, max, eng), rand(min, max, eng) };
}

inline float rand01(std::mt19937_64& eng)
{
	return std::uniform_real_distribution<float>(0.0f, 1.0f)(eng);
}

inline GFX::Data::ColorFloat4 randColor(std::mt19937_64& eng)
{
	return { rand01(eng), rand01(eng), rand01(eng) };
}

inline DirectX::XMFLOAT3 normalize(DirectX::XMFLOAT3 v)
{
	DirectX::XMStoreFloat3(&v, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&v)));
	return std::move(v);
}

DirectX::XMFLOAT3 add(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2);

DirectX::XMFLOAT3 addNormal(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2);
