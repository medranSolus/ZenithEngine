#pragma once
#include "ObjectBase.h"

namespace GFX::Object
{
	class Rectangle : public ObjectBase<Rectangle>
	{
		DirectX::XMFLOAT3 angle = { 0.0f,0.0f,0.0f };
		DirectX::XMFLOAT3 pos;
		float width;
		float height;

	public:
		Rectangle(Graphics & gfx, float x0, float y0, float z0, float width, float height, bool isRandom = false);

		constexpr DirectX::XMFLOAT3 GetPos() const noexcept { return pos; }
		constexpr void SetPos(DirectX::XMFLOAT3 position) noexcept { pos = position; }
		constexpr DirectX::XMFLOAT3 GetAngle() const noexcept { return angle; }
		constexpr void SetAngle(DirectX::XMFLOAT3 meshAngle) noexcept { angle = meshAngle; }

		void Update(float dX, float dY, float dZ, float angleDZ = 0.0f, float angleDX = 0.0f, float angleDY = 0.0f) noexcept override;
		DirectX::XMMATRIX GetTransformMatrix() const noexcept override;
	};
}
