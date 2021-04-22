#pragma once
#include "Types.h"
#include <utility>

// RGB 3 float channel
class ColorF3 final
{
public:
	Float3 RGB = { 0.0f, 0.0f, 0.0f };

	ColorF3() = default;
	constexpr ColorF3(float r, float g, float b) noexcept : RGB(r, g, b) {}
	constexpr ColorF3(Float3&& rgb) noexcept : RGB(std::move(rgb)) {}
	constexpr ColorF3(const Float3& rgb) noexcept : RGB(rgb) {}
	ColorF3(ColorF3&&) = default;
	ColorF3(const ColorF3&) = default;
	ColorF3& operator=(ColorF3&&) = default;
	ColorF3& operator=(const ColorF3&) = default;
	~ColorF3() = default;

	constexpr operator Float3() const noexcept { return RGB; }
	bool operator==(const ColorF3& c) const noexcept { return Math::XMVector3Equal(Math::XMLoadFloat3(&c.RGB), Math::XMLoadFloat3(&RGB)); }
	bool operator!=(const ColorF3& c) const noexcept { return Math::XMVector3NotEqual(Math::XMLoadFloat3(&c.RGB), Math::XMLoadFloat3(&RGB)); }
	ColorF3 operator-() const { return *this * -1.0f; }

	ColorF3& operator-=(const ColorF3& c) noexcept { return *this = *this - c; }
	ColorF3& operator+=(const ColorF3& c) noexcept { return *this = *this + c; }
	ColorF3& operator*=(float x) noexcept { return *this = *this * x; }

	ColorF3 operator-(const ColorF3& c) const noexcept;
	ColorF3 operator+(const ColorF3& c) const noexcept;
	ColorF3 operator*(float x) const noexcept;
};