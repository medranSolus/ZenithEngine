#pragma once
#include "ObjectBase.h"

namespace GFX::Object
{
	class Ball : public ObjectBase<Ball>
	{
		DirectX::XMFLOAT3 angle = { 0.0f,0.0f,0.0f };
		DirectX::XMFLOAT3 pos;
		float r;

	public:
		Ball(Graphics & gfx, BasicType::ColorFloat material, float x0, float y0, float z0, unsigned int density, float radius);

		constexpr DirectX::XMFLOAT3 GetPos() const noexcept { return pos; }
		constexpr void SetPos(DirectX::XMFLOAT3 position) noexcept { pos = position; }
		constexpr DirectX::XMFLOAT3 GetAngle() const noexcept { return angle; }
		constexpr void SetAngle(DirectX::XMFLOAT3 meshAngle) noexcept { angle = meshAngle; }

		void Update(float dX, float dY, float dZ, float angleDZ = 0.0f, float angleDX = 0.0f, float angleDY = 0.0f) noexcept override;
		DirectX::XMMATRIX GetTransformMatrix() const noexcept override;
	};
}
