#include "Color.h"

namespace GFX::BasicType
{
	ColorFloat ColorFloat::operator+(const ColorFloat & c) const
	{
		ColorFloat nc;
		DirectX::XMStoreFloat4(&nc.col, DirectX::XMVectorSaturate(DirectX::XMVectorAdd(DirectX::XMLoadFloat4(&col), DirectX::XMLoadFloat4(&c.col))));
		return std::move(nc);
	}
}
