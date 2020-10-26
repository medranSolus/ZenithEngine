#include "ConstBufferTransformEx.h"

namespace GFX::Resource
{
    DirectX::XMFLOAT3 ConstBufferTransformEx::GetPos() const noexcept
    {
        DirectX::XMFLOAT4X4 transform;
        DirectX::XMStoreFloat4x4(&transform, GetTransform());
        return { transform._13, transform._23, transform._33 };
    }
}