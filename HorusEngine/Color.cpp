#include "Color.h"

namespace GFX::BasicType
{
	ColorFloat& ColorFloat::operator=(const ColorByte& c) noexcept
	{
		col.x = c.r / 255.0f;
		col.y = c.g / 255.0f;
		col.z = c.b / 255.0f;
		col.w = c.a / 255.0f;
		return *this;
	}

	ColorFloat ColorFloat::operator+(const ColorFloat& c) const
	{
		ColorFloat nc;
		DirectX::XMStoreFloat4(&nc.col, DirectX::XMVectorSaturate(DirectX::XMVectorAdd(DirectX::XMLoadFloat4(&col), DirectX::XMLoadFloat4(&c.col))));
		return std::move(nc);
	}
}