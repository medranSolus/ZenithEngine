#pragma once
#include "ColorF4.h"
#include <array>
#include <random>

namespace ZE::Math
{
	constexpr U64 GIGABYTE = 1ULL << 30;
	constexpr U64 MEGABYTE = 1ULL << 20;
	constexpr U64 KILOBYTE = 1ULL << 10;
	constexpr float PI = static_cast<float>(M_PI);
	constexpr float PI2 = static_cast<float>(M_PI * 2.0);

	constexpr Float4 NoRotation() noexcept { return { 0.0f, 0.0f, 0.0f, 1.0f }; }
	constexpr Float3 NoRotationAngles() noexcept { return { 0.0f, 0.0f, 0.0f }; }
	constexpr Float3 StartPosition() noexcept { return { 0.0f, 0.0f, 0.0f }; }
	constexpr Float3 UnitScale() noexcept { return { 1.0f, 1.0f, 1.0f }; }

	// Filter to use during image processing
	enum class FilterType : U8
	{
		Box = 0,
		GammaAverage = 1,
		Bilinear = 2,
		Kaiser = 3,
		Lanczos = 4,
		Gauss = 5,
	};

	// Used for traversal of corresponding cubemap face in 3D space
	struct CubemapFaceTraversalDesc
	{
		constexpr static float POINT = 0.5f;

		Float3 StartPos;
		Float3 DirX;
		Float3 DirY;
	};

	// +x, -x, +y, -y, +z, -z
	constexpr std::array<CubemapFaceTraversalDesc, 6> CUBEMAP_FACES_INFO =
	{ {
		{ { -CubemapFaceTraversalDesc::POINT, CubemapFaceTraversalDesc::POINT, CubemapFaceTraversalDesc::POINT }, { 0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f, 0.0f } },
		{ { CubemapFaceTraversalDesc::POINT, CubemapFaceTraversalDesc::POINT, -CubemapFaceTraversalDesc::POINT }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
		{ { CubemapFaceTraversalDesc::POINT, CubemapFaceTraversalDesc::POINT, -CubemapFaceTraversalDesc::POINT }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
		{ { CubemapFaceTraversalDesc::POINT, -CubemapFaceTraversalDesc::POINT, CubemapFaceTraversalDesc::POINT }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
		{ { CubemapFaceTraversalDesc::POINT, CubemapFaceTraversalDesc::POINT, CubemapFaceTraversalDesc::POINT }, { -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
		{ { -CubemapFaceTraversalDesc::POINT, CubemapFaceTraversalDesc::POINT, -CubemapFaceTraversalDesc::POINT }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
	} };

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

	constexpr S16 ClampAngle(S16 angle, S16 maxAngle = 360) noexcept
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

	inline U16 GetMipLevels(U32 width, U32 height) noexcept
	{
		return static_cast<U16>(std::floor(std::log2(std::max(width, height)))) + 1;
	}

	inline float Wrap(float x, float wrap) noexcept
	{
		return static_cast<float>(std::fmod(x, wrap));
	}

	inline float Wrap2Pi(float x) noexcept
	{
		return Wrap(x, PI2);
	}

	inline float Wrap2PiRand(std::mt19937_64& eng) noexcept
	{
		return std::uniform_real_distribution<float>(0.0f, PI2)(eng);
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

	inline Float3 NormalizeReturn(const Float3& v) noexcept
	{
		Float3 n;
		XMStoreFloat3(&n, XMVector3Normalize(XMLoadFloat3(&v)));
		return n;
	}

	inline Float3& NormalizeStore(Float3& v) noexcept
	{
		XMStoreFloat3(&v, XMVector3Normalize(XMLoadFloat3(&v)));
		return v;
	}

	inline Float4 GetQuaternion(float angleX, float angleY, float angleZ) noexcept
	{
		Float4 q;
		XMStoreFloat4(&q, XMQuaternionRotationRollPitchYaw(ToRadians(angleX), ToRadians(angleY), ToRadians(angleZ)));
		return q;
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
	constexpr T DivideRoundUp(T x, T y) noexcept
	{
		return (x + y - static_cast<T>(1)) / y;
	}

	template<typename T>
	constexpr T Kaiser(T x, T alpha, T length) noexcept
	{
		if (x <= length && x >= 0)
		{
			T piA = static_cast<T>(M_PI) * alpha;
			T besselArg = piA * std::sqrt(static_cast<T>(1) - std::pow(static_cast<T>(2) * x / length - 1, static_cast<T>(2)));
			return std::cyl_bessel_i(alpha, besselArg) / std::cyl_bessel_i(alpha, piA);
		}
		return static_cast<T>(0);
	}

	template<typename T>
	constexpr T Lanczos(T x, T length) noexcept
	{
		if (std::abs(x) < FLT_EPSILON)
			return static_cast<T>(1);
		T piX = static_cast<T>(M_PI) * x;
		return length * std::sin(piX) * std::sin(piX / length) / (piX * piX);
	}

	template<typename T>
	constexpr T Gauss(T x, T sigma) noexcept
	{
		return static_cast<T>(M_2_SQRTPI) * std::exp(static_cast<T>(-0.5) * x * x / sigma) / (sigma * static_cast<T>(M_SQRT2 * 2.0));
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

	template<typename T>
	constexpr T Sign(T val) noexcept
	{
		return static_cast<T>((static_cast<T>(0) < val) - (val < static_cast<T>(0)));
	}

	template<typename T>
	constexpr T WrapCoord(T coord, T size) noexcept
	{
		return (coord % size + size) % size;
	}

	template<typename T>
	constexpr T MirrorCoord(T coord, T size) noexcept
	{
		const T mod = coord % (size * 2);
		return mod < size ? (mod < 0 ? std::abs(mod + 1) : mod) : (size * 2 - mod - 1);
	}

	Float3 GetEulerAngles(const Float4& rotor) noexcept;
	Float3 Add(const Float3& v1, const Float3& v2) noexcept;
	Float3 AddNormal(const Float3& v1, const Float3& v2) noexcept;
	bool IsNearEqual(const Vector& v1, const Vector& v2) noexcept;
	bool IsUnitQuaternion(const Vector& rotor) noexcept;
	Matrix GetVectorRotation(const Vector& baseDirection, const Vector& newDirection,
		bool targetGeometry = false, float geometryOffsetY = 0.0f) noexcept;
	Matrix GetTransform(const Float3& position, const Float4& rotor, const Float3& scale) noexcept;
	BoundingBox GetBoundingBox(const Vector& maxPositive, const Vector& maxNegative) noexcept;
	// Samples cubemap in given direction, returns texel coordinates in cubemap face (X, Y, face index)
	UInt3 SampleCubemap(const Vector& direction, U32 cubemapSize) noexcept;
	// Number of filter coefficients must match ceil(sqrt(samples.size()) / 2)
	Vector ApplyFilter(FilterType filter, std::vector<Float4>& samples, float bilinearFactorX = 0.5f, float bilinearFactorY = 0.5f, const std::vector<float>* filterCoeff = nullptr) noexcept;
}