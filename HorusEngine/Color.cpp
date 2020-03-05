#include "Color.h"

namespace GFX::Data
{
	ColorFloat4& ColorFloat4::operator=(const ColorByte& c) noexcept
	{
		col.x = c.r / 255.0f;
		col.y = c.g / 255.0f;
		col.z = c.b / 255.0f;
		col.w = c.a / 255.0f;
		return *this;
	}

	ColorFloat4 ColorFloat4::operator+(const ColorFloat4& c) const
	{
		ColorFloat4 nc;
		DirectX::XMStoreFloat4(&nc.col, DirectX::XMVectorSaturate(DirectX::XMVectorAdd(DirectX::XMLoadFloat4(&col), DirectX::XMLoadFloat4(&c.col))));
		return std::move(nc);
	}
}