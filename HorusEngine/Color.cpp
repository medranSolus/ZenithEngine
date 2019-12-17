#include "Color.h"

namespace GFX::BasicType
{
	ColorByte & ColorByte::operator=(const ColorByte & c)
	{
		r = c.r;
		g = c.g;
		b = c.b;
		a = c.a;
		return *this;
	}

	ColorByte ColorByte::operator+(const ColorByte & c) const
	{
		return { static_cast<unsigned char>((r + c.r) >> 1),
			static_cast<unsigned char>((g + c.g) >> 1),
			static_cast<unsigned char>((b + c.b) >> 1),
			static_cast<unsigned char>((a + c.a) >> 1) };
	}

	ColorFloat & ColorFloat::operator=(const ColorFloat & c)
	{
		col = c.col;
		return *this;
	}

	ColorFloat & ColorFloat::operator=(ColorFloat && c)
	{
		col = std::move(c.col);
		return *this;
	}

	ColorFloat ColorFloat::operator+(const ColorFloat & c) const
	{
		ColorFloat nc;
		DirectX::XMStoreFloat4(&nc.col, DirectX::XMVectorSaturate(DirectX::XMVectorAdd(DirectX::XMLoadFloat4(&col), DirectX::XMLoadFloat4(&c.col))));
		return std::move(nc);
	}
}
