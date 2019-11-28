#pragma once
#include "ObjectBase.h"

namespace GFX::Object
{
	class Triangle : public ObjectBase<Triangle>
	{
		float angleZ = 0.0f;
		float x;
		float y;
		float z;
		
	public:
		Triangle(Graphics & gfx, float x0, float y0, float z0, float down, float left, float right);

		void Update(float dX, float dY, float dZ, float angleDZ = 0.0f, float angleDX = 0.0f, float angleDY = 0.0f) noexcept override;
		DirectX::XMMATRIX GetTransformMatrix() const noexcept override;
	};
}