#pragma once
#include "Types.h"
#include <utility>

namespace Data
{
	// RGBA 4 float channel
	class ColorF4 final
	{
	public:
		Float4 RGBA = { 0.0f, 0.0f, 0.0f, 1.0f };

		ColorF4() = default;
		constexpr ColorF4(float r, float g, float b, float a = 1.0f) noexcept : RGBA(r, g, b, a) {}
		constexpr ColorF4(Float4&& rgba) noexcept : RGBA(std::move(rgba)) {}
		constexpr ColorF4(const Float4& rgba) noexcept : RGBA(rgba) {}
		ColorF4(ColorF4&&) = default;
		ColorF4(const ColorF4&) = default;
		ColorF4& operator=(ColorF4&&) = default;
		ColorF4& operator=(const ColorF4&) = default;
		~ColorF4() = default;

		constexpr operator Float4() const noexcept { return RGBA; }
		bool operator==(const ColorF4& c) const { return Math::XMVector4Equal(Math::XMLoadFloat4(&c.RGBA), Math::XMLoadFloat4(&RGBA)); }
		bool operator!=(const ColorF4& c) const { return Math::XMVector4NotEqual(Math::XMLoadFloat4(&c.RGBA), Math::XMLoadFloat4(&RGBA)); }
		ColorF4 operator-() const { return *this * -1.0f; }

		ColorF4& operator-=(const ColorF4& c) noexcept { return *this = *this - c; }
		ColorF4& operator+=(const ColorF4& c) noexcept { return *this = *this + c; }
		ColorF4& operator*=(float x) noexcept { return *this = *this * x; }

		ColorF4 operator-(const ColorF4& c) const;
		ColorF4 operator+(const ColorF4& c) const;
		ColorF4 operator*(float x) const;
	};
}