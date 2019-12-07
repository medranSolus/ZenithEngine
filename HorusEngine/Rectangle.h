#pragma once
#include "ObjectBase.h"

namespace GFX::Object
{
	class Rectangle : public ObjectBase<Rectangle>
	{
		float width;
		float height;

	public:
		Rectangle(Graphics & gfx, float x0, float y0, float z0, float width, float height, bool isRandom = false);

		void Update(float dX, float dY, float dZ, float angleDZ = 0.0f, float angleDX = 0.0f, float angleDY = 0.0f) noexcept override;
		DirectX::XMMATRIX GetTransformMatrix() const noexcept override;
	};
}
