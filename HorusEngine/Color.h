#pragma once
#include <DirectXMath.h>
#include <utility>

namespace GFX::BasicType
{
	class ColorByte
	{
	public:
		unsigned char r = 0;
		unsigned char g = 0;
		unsigned char b = 0;
		unsigned char a = 255;

		ColorByte() = default;
		constexpr ColorByte(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) noexcept : r(r), g(g), b(b), a(a) {}
		constexpr ColorByte(const ColorByte & c) noexcept : r(c.r), g(c.g), b(c.b), a(c.a) {}
		constexpr ColorByte & operator=(const ColorByte & c) noexcept;

		constexpr ColorByte operator+(const ColorByte & c) const noexcept;
	};

	class ColorFloat
	{
	public:
		DirectX::XMFLOAT4 col = { 0.0f,0.0f,0.0f,1.0f };

		ColorFloat() = default;
		constexpr ColorFloat(float r, float g, float b, float a = 1.0f) noexcept : col(r, g, b, a) {}

		constexpr ColorFloat(const DirectX::XMFLOAT4 & col) noexcept : col(col) {}
		constexpr ColorFloat(DirectX::XMFLOAT4 && col) noexcept : col(std::move(col)) {}

		constexpr ColorFloat(const ColorFloat & c) noexcept : col(c.col) {}
		constexpr ColorFloat(ColorFloat && c) noexcept : col(std::move(c.col)) {}

		constexpr ColorFloat(const ColorByte & c) noexcept : col(c.r, c.g, c.b, c.a) {}

		constexpr ColorFloat & operator=(const ColorFloat & c) noexcept;
		constexpr ColorFloat & operator=(ColorFloat && c) noexcept;

		ColorFloat operator+(const ColorFloat & c) const;
	};

	constexpr ColorByte & ColorByte::operator=(const ColorByte & c) noexcept
	{
		r = c.r;
		g = c.g;
		b = c.b;
		a = c.a;
		return *this;
	}

	constexpr ColorByte ColorByte::operator+(const ColorByte & c) const noexcept
	{
		return { static_cast<unsigned char>((r + c.r) >> 1),
			static_cast<unsigned char>((g + c.g) >> 1),
			static_cast<unsigned char>((b + c.b) >> 1),
			static_cast<unsigned char>((a + c.a) >> 1) };
	}

	constexpr ColorFloat & ColorFloat::operator=(const ColorFloat & c) noexcept
	{
		col = c.col;
		return *this;
	}

	constexpr ColorFloat & ColorFloat::operator=(ColorFloat && c) noexcept
	{
		col = std::move(c.col);
		return *this;
	}
}
