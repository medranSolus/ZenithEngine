#pragma once
#define _USE_MATH_DEFINES
#include "Color.h"
#include <math.h>
#include <random>

namespace Math
{
	constexpr short ClampAngle(short angle, short maxAngle = 360) noexcept
	{
		if (angle > maxAngle)
			return maxAngle;
		else if (angle < -maxAngle)
			return -maxAngle;
		return angle;
	}

	constexpr bool Equals(float a, float b) noexcept
	{
		return a >= b - FLT_EPSILON && a <= b + FLT_EPSILON;
	}

	constexpr bool NotEquals(float a, float b) noexcept
	{
		return a <= b - FLT_EPSILON || a >= b + FLT_EPSILON;
	}

	inline float Wrap2Pi(float x) noexcept
	{
		return static_cast<float>(fmod(x, 2.0f * M_PI));
	}

	inline float Wrap(float x, float wrap) noexcept
	{
		return static_cast<float>(fmod(x, wrap));
	}

	inline float Wrap2PiRand(std::mt19937_64& eng) noexcept
	{
		return std::uniform_real_distribution<float>(0.0f, 2.0f * M_PI)(eng);
	}

	inline float WrapNDCRand(std::mt19937_64& eng) noexcept
	{
		return std::uniform_real_distribution<float>(-1.0f, 1.0f)(eng);
	}

	inline float Rand(float min, float max, std::mt19937_64& eng) noexcept
	{
		return std::uniform_real_distribution<float>(min, max)(eng);
	}

	inline DirectX::XMFLOAT3 RandPosition(float min, float max, std::mt19937_64& eng) noexcept
	{
		return { Rand(min, max, eng), Rand(min, max, eng), Rand(min, max, eng) };
	}

	inline float Rand01(std::mt19937_64& eng) noexcept
	{
		return std::uniform_real_distribution<float>(0.0f, 1.0f)(eng);
	}

	inline float RandNDC(std::mt19937_64& eng) noexcept
	{
		return std::uniform_real_distribution<float>(-1.0f, 1.0f)(eng);
	}

	inline GFX::Data::ColorFloat4 RandColor(std::mt19937_64& eng) noexcept
	{
		return { Rand01(eng), Rand01(eng), Rand01(eng) };
	}

	inline DirectX::XMVECTOR Normalize(const DirectX::XMFLOAT3& v) noexcept
	{
		return DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&v));
	}

	inline DirectX::XMFLOAT3& NormalizeStore(DirectX::XMFLOAT3& v) noexcept
	{
		DirectX::XMStoreFloat3(&v, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&v)));
		return v;
	}

	template<typename T>
	constexpr T Gauss(T x, T sigma) noexcept
	{
		return static_cast<T>(M_2_SQRTPI) * exp(static_cast<T>(-0.5) * x * x / sigma) / (sigma * static_cast<T>(M_SQRT2 * 2.0));
	}

	template<typename T>
	constexpr T Lerp(T a, T b, T weight) noexcept
	{
		return a + weight * (b - a);
	}

	DirectX::XMFLOAT3 Add(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2) noexcept;
	DirectX::XMFLOAT3 AddNormal(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2) noexcept;
	bool IsNearEqual(const DirectX::XMVECTOR& v1, const DirectX::XMVECTOR& v2) noexcept;
	DirectX::XMMATRIX GetVectorRotation(const DirectX::XMVECTOR& baseDirection, const DirectX::XMVECTOR& newDirection,
		bool targetGeometry = false, float geometryOffsetY = 0.0f) noexcept;
}