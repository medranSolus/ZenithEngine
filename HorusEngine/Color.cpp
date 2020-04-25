#include "Color.h"

namespace GFX::Data
{
	ColorFloat3& ColorFloat3::operator=(const ColorByte& c) noexcept
	{
		col.x = c.r / 255.0f;
		col.y = c.g / 255.0f;
		col.z = c.b / 255.0f;
		return *this;
	}

	ColorFloat3 ColorFloat3::operator+(const ColorFloat3& c) const
	{
		ColorFloat3 nc;
		DirectX::XMStoreFloat3(&nc.col, DirectX::XMVectorSaturate(DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&col), DirectX::XMLoadFloat3(&c.col))));
		return std::move(nc);
	}

	ColorFloat4& ColorFloat4::operator=(const ColorByte& c) noexcept
	{
		col.x = c.r / 255.0f;
		col.y = c.g / 255.0f;
		col.z = c.b / 255.0f;
		col.w = c.a / 255.0f;
		return *this;
	}

	ColorFloat4& ColorFloat4::operator=(const ColorFloat3& c) noexcept
	{
		col.x = c.col.x;
		col.y = c.col.y;
		col.z = c.col.z;
		col.w = 1.0f;
		return *this;
	}

	ColorFloat4 ColorFloat4::operator+(const ColorFloat4& c) const
	{
		ColorFloat4 nc;
		DirectX::XMStoreFloat4(&nc.col, DirectX::XMVectorSaturate(DirectX::XMVectorAdd(DirectX::XMLoadFloat4(&col), DirectX::XMLoadFloat4(&c.col))));
		return std::move(nc);
	}
}