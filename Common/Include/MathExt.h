#pragma once
#include "ColorF4.h"
#include <random>

namespace ZE::Math
{
	constexpr float ToRadians(float angle) noexcept
	{
		return static_cast<float>(M_PI - FLT_EPSILON) * angle / 180.0f;
	}

	constexpr float ToDegrees(float angle) noexcept
	{
		return angle * 180.0f / static_cast<float>(M_PI + FLT_EPSILON);
	}

	constexpr Float3 ToRadians(Float3 angles) noexcept
	{
		return { ToRadians(angles.x), ToRadians(angles.y), ToRadians(angles.z) };
	}

	constexpr Float3 ToDegrees(Float3 angles) noexcept
	{
		return { ToDegrees(angles.x), ToDegrees(angles.y), ToDegrees(angles.z) };
	}

	constexpr short ClampAngle(U16 angle, U16 maxAngle = 360) noexcept
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
		return std::uniform_real_distribution<float>(0.0f, 2.0f * static_cast<float>(M_PI))(eng);
	}

	inline float WrapNDCRand(std::mt19937_64& eng) noexcept
	{
		return std::uniform_real_distribution<float>(-1.0f, 1.0f)(eng);
	}

	inline float Rand(float min, float max, std::mt19937_64& eng) noexcept
	{
		return std::uniform_real_distribution<float>(min, max)(eng);
	}

	inline Float3 RandPosition(float min, float max, std::mt19937_64& eng) noexcept
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

	inline ColorF4 RandColor(std::mt19937_64& eng) noexcept
	{
		return { Rand01(eng), Rand01(eng), Rand01(eng) };
	}

	inline Vector Normalize(const Float3& v) noexcept
	{
		return XMVector3Normalize(XMLoadFloat3(&v));
	}

	inline Float3& NormalizeStore(Float3& v) noexcept
	{
		XMStoreFloat3(&v, XMVector3Normalize(XMLoadFloat3(&v)));
		return v;
	}

	inline float GetLightVolume(const ColorF3& color, float intensity, float attnLinear, float attnQuad) noexcept
	{
		const float lightMax = intensity * fmaxf(fmaxf(color.RGB.x, color.RGB.y), color.RGB.z);
		return (-attnLinear + sqrtf(attnLinear * attnLinear - 4.0f * attnQuad * (1.0f - lightMax * 256.0f))) / (2.0f * attnQuad);
	}

	// Not to be used with floats
	template<typename T>
	constexpr bool IsPower2(T x) noexcept
	{
		return x && !(x & (x - 1));
	}

	// Not to be used with floats
	template<typename T>
	constexpr T AlignUp(T val, T alignment) noexcept
	{
		ZE_ASSERT(IsPower2(alignment), "Incorrect alignment!");
		return (val + alignment - 1) & ~(alignment - 1);
	}

	// Not to be used with floats
	template<typename T>
	constexpr T AlignDown(T val, T alignment) noexcept
	{
		ZE_ASSERT(IsPower2(alignment), "Incorrect alignment!");
		return val & ~(alignment - 1);
	}

	template <typename T>
	constexpr T DivideRoundUp(T x, T y)
	{
		return (x + y - static_cast<T>(1)) / y;
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

	template<typename T>
	constexpr T Clamp(T val, T down, T up) noexcept
	{
		return val < down ? down : (val > up ? up : val);
	}

	Float3 GetEulerAngles(const Float4& rotor) noexcept;
	Float3 Add(const Float3& v1, const Float3& v2) noexcept;
	Float3 AddNormal(const Float3& v1, const Float3& v2) noexcept;
	bool IsNearEqual(const Vector& v1, const Vector& v2) noexcept;
	Matrix GetVectorRotation(const Vector& baseDirection, const Vector& newDirection,
		bool targetGeometry = false, float geometryOffsetY = 0.0f) noexcept;
	Matrix GetTransform(const Float3& position, const Float4& rotor, const Float3& scale) noexcept;
}