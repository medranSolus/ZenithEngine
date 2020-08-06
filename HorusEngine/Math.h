#pragma once
#define _USE_MATH_DEFINES
#include "Vertex.h"
#include <math.h>
#include <random>

constexpr bool equals(float a, float b) noexcept
{
	return a >= b - FLT_EPSILON && a <= b + FLT_EPSILON;
}

constexpr bool notEquals(float a, float b) noexcept
{
	return a <= b - FLT_EPSILON || a >= b + FLT_EPSILON;
}

inline float wrap2Pi(float x) noexcept
{
	return static_cast<float>(fmod(x, 2.0f * M_PI));
}

inline float wrap(float x, float wrap) noexcept
{
	return static_cast<float>(fmod(x, wrap));
}

inline float randWrap2Pi(std::mt19937_64& eng) noexcept
{
	return std::uniform_real_distribution<float>(0.0f, 2.0f * M_PI)(eng);
}

inline float randWrapNDC(std::mt19937_64& eng) noexcept
{
	return std::uniform_real_distribution<float>(-1.0f, 1.0f)(eng);
}

inline float rand(float min, float max, std::mt19937_64& eng) noexcept
{
	return std::uniform_real_distribution<float>(min, max)(eng);
}

inline DirectX::XMFLOAT3 randPosition(float min, float max, std::mt19937_64& eng) noexcept
{
	return { rand(min, max, eng), rand(min, max, eng), rand(min, max, eng) };
}

inline float rand01(std::mt19937_64& eng) noexcept
{
	return std::uniform_real_distribution<float>(0.0f, 1.0f)(eng);
}

inline GFX::Data::ColorFloat4 randColor(std::mt19937_64& eng) noexcept
{
	return { rand01(eng), rand01(eng), rand01(eng) };
}

inline DirectX::XMFLOAT3 normalize(DirectX::XMFLOAT3 v) noexcept
{
	DirectX::XMStoreFloat3(&v, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&v)));
	return std::move(v);
}

template<typename T>
constexpr T gauss(T x, T sigma) noexcept
{
	return static_cast<T>(M_2_SQRTPI) * exp(static_cast<T>(-0.5) * x * x / sigma) / (sigma * static_cast<T>(M_SQRT2 * 2.0));
}

DirectX::XMFLOAT3 add(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2);

DirectX::XMFLOAT3 addNormal(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2);