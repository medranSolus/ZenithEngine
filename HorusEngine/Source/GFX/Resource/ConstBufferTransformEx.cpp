#include "GFX/Resource/ConstBufferTransformEx.h"

namespace GFX::Resource
{
	ConstBufferTransformEx::ConstBufferTransformEx(Graphics& gfx, const GfxObject& parent, U32 slot)
		: ConstBufferTransform(gfx, parent, slot)
	{
		Math::XMStoreFloat4x4(&transform, Math::XMMatrixIdentity());
	}

	Float3 ConstBufferTransformEx::GetPos() const noexcept
	{
		Float4x4 transform;
		Math::XMStoreFloat4x4(&transform, GetTransform());
		return { transform._13, transform._23, transform._33 };
	}
}