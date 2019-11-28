#pragma once
#include "ObjectBase.h"

namespace GFX::Object
{
	class Box : public ObjectBase<Box>
	{
		float r;
		float rotZ = 0.0f;
		float rotX = 0.0f;
		float rotY = 0.0f;
		float moveX;
		float moveY;
		float moveZ;

		float dRotX;
		float dRotY;
		float dRotZ;
		float dMoveX;
		float dMoveY;
		float dMoveZ;

	public:
		Box(Graphics & gfx, float x0, float y0, float z0, float rotationR);

		void Update(float dX, float dY, float dZ, float angleDZ = 0.0f, float angleDX = 0.0f, float angleDY = 0.0f) noexcept override;
		DirectX::XMMATRIX GetTransformMatrix() const noexcept override;
	};
}
