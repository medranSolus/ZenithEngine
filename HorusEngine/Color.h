#pragma once
#include <DirectXMath.h>
#include <utility>

namespace GFX::BasicType
{
	class ColorByte
	{
	public:
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;

		ColorByte(unsigned char r = 0, unsigned char g = 0, unsigned char b = 0, unsigned char a = 255) : r(r), g(g), b(b), a(a) {}
		ColorByte(const ColorByte & c) : r(c.r), g(c.g), b(c.b), a(c.a) {}
		ColorByte & operator=(const ColorByte & c);

		ColorByte operator+(const ColorByte & c) const;
	};

	class ColorFloat
	{
	public:
		DirectX::XMFLOAT4 col;

		ColorFloat(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f) : col(r, g, b, a) {}
		ColorFloat(const DirectX::XMFLOAT4 & col) : col(col) {}
		ColorFloat(DirectX::XMFLOAT4 && col) : col(std::move(col)) {}
		ColorFloat(const ColorFloat & c) : col(c.col) {}
		ColorFloat(ColorFloat && c) : col(std::move(c.col)) {}
		ColorFloat(const ColorByte & c) : col(c.r, c.g, c.b, c.a) {}
		ColorFloat & operator=(const ColorFloat & c);
		ColorFloat & operator=(ColorFloat && c);

		ColorFloat operator+(const ColorFloat & c) const;
	};
}
