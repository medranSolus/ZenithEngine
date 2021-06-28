#include "GFX/Data/BoundingBox.h"

namespace ZE::GFX::Data
{
	BoundingBox BoundingBox::dummy;

	void BoundingBox::Finalize() noexcept
	{
		const Vector half = Math::XMVectorSet(0.5f, 0.5f, 0.5f, 0.0f);
		const Vector positive = Math::XMLoadFloat3(&box.Center);
		const Vector negative = Math::XMLoadFloat3(&box.Extents);
		Math::XMStoreFloat3(&box.Extents,
			Math::XMVectorMultiply(Math::XMVectorSubtract(positive, negative), half));
		Math::XMStoreFloat3(&box.Center,
			Math::XMVectorMultiply(Math::XMVectorAdd(positive, negative), half));
	}

	bool BoundingBox::Intersects(const Math::BoundingFrustum& frustum, const Matrix& transform) const noexcept
	{
		External::DirectX::BoundingBox transformed;
		box.Transform(transformed, transform);
		return frustum.Intersects(transformed);
	}
}