#pragma once
#include "ObjectBase.h"

namespace GFX::Object
{
	class Box : public ObjectBase<Box>
	{
		DirectX::XMFLOAT3 rotationScale;
		DirectX::XMFLOAT3 posScale;
		float r;

	public:
		Box(Graphics & gfx, float x0, float y0, float z0, float rotationR);

		void Update(float dX, float dY, float dZ, float angleDZ = 0.0f, float angleDX = 0.0f, float angleDY = 0.0f) noexcept override;
		DirectX::XMMATRIX GetTransformMatrix() const noexcept override;
	};
}
