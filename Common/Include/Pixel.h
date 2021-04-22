#pragma once
#include "Types.h"

// RGBA 4 byte pixel
class Pixel final
{
public:
	U8 Red = 0;
	U8 Green = 0;
	U8 Blue = 0;
	U8 Alpha = 255;

	Pixel() = default;
	constexpr Pixel(U32 rgba) noexcept;
	constexpr Pixel(U8 r, U8 g, U8 b, U8 a = 255) noexcept : Red(r), Green(g), Blue(b), Alpha(a) {}
	Pixel(Pixel&&) = default;
	Pixel(const Pixel&) = default;
	Pixel& operator=(Pixel&&) = default;
	Pixel& operator=(const Pixel&) = default;
	~Pixel() = default;

	constexpr U32 GetValue() const noexcept { return Red | (Green << 8) | (Blue << 16) | (Alpha << 24); }
	constexpr operator U32() const noexcept { return GetValue(); }

	constexpr bool operator==(const Pixel& c) const noexcept { return static_cast<const U32&>(*this) == static_cast<const U32&>(c); }
	constexpr bool operator!=(const Pixel& c) const noexcept { return !(*this == c); }
	constexpr Pixel operator-(const Pixel& c) const noexcept { return *this + -c; }

	constexpr Pixel& operator-=(const Pixel& c) noexcept { return *this = *this - c; }
	constexpr Pixel& operator+=(const Pixel& c) noexcept { return *this = *this + c; }
	constexpr Pixel& operator*=(float x) noexcept { return *this = *this * x; }

	constexpr Pixel operator-() const noexcept;
	constexpr Pixel operator+(const Pixel& c) const noexcept;
	constexpr Pixel operator*(float x) const noexcept;
};

#pragma region Functions
constexpr Pixel::Pixel(U32 rgba) noexcept
	: Red(rgba & 0xFF), Green((rgba >> 8) & 0xFF), Blue((rgba >> 16) & 0xFF), Alpha(rgba >> 24) {}

constexpr Pixel Pixel::operator-() const noexcept
{
	return
	{
		static_cast<U8>(255 - Red),
		static_cast<U8>(255 - Green),
		static_cast<U8>(255 - Blue),
		static_cast<U8>(255 - Alpha)
	};
}

constexpr Pixel Pixel::operator+(const Pixel& c) const noexcept
{
	return
	{
		static_cast<U8>(Red + c.Red),
		static_cast<U8>(Green + c.Green),
		static_cast<U8>(Blue + c.Blue),
		static_cast<U8>(Alpha + c.Alpha)
	};
}

constexpr Pixel Pixel::operator*(float x) const noexcept
{
	return
	{
		static_cast<U8>(Red * x),
		static_cast<U8>(Green * x),
		static_cast<U8>(Blue * x),
		static_cast<U8>(Alpha * x)
	};
}
#pragma endregion