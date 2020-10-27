#include "BoundingBox.h"

namespace GFX::Data
{
    BoundingBox BoundingBox::dummy;

    void BoundingBox::Finalize() noexcept
    {
        const DirectX::XMVECTOR half = DirectX::XMVectorSet(0.5f, 0.5f, 0.5f, 0.0f);
        const DirectX::XMVECTOR positive = DirectX::XMLoadFloat3(&box.Center);
        const DirectX::XMVECTOR negative = DirectX::XMLoadFloat3(&box.Extents);
        DirectX::XMStoreFloat3(&box.Extents, DirectX::XMVectorMultiply(DirectX::XMVectorSubtract(positive, negative), half));
        DirectX::XMStoreFloat3(&box.Center, DirectX::XMVectorMultiply(DirectX::XMVectorAdd(positive, negative), half));
    }

    bool BoundingBox::Intersects(const DirectX::BoundingFrustum& frustum, const DirectX::XMMATRIX& transform) const noexcept
    {
        DirectX::BoundingBox transformed;
        box.Transform(transformed, transform);
        return frustum.Intersects(transformed);
    }
}