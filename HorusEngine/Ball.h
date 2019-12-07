#pragma once
#include "ObjectBase.h"

namespace GFX::Object
{
	class Ball : public ObjectBase<Ball>
	{
		float r;

	public:
		Ball(Graphics & gfx, float x0, float y0, float z0, unsigned int density, float radius);

		void Update(float dX, float dY, float dZ, float angleDZ = 0.0f, float angleDX = 0.0f, float angleDY = 0.0f) noexcept override;
		DirectX::XMMATRIX GetTransformMatrix() const noexcept override;
	};
}
