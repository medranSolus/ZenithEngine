#pragma once
#include "BaseShape.h"
#include "Object.h"

namespace GFX::Shape
{
	class Box : public BaseShape<Box>, public Object
	{
		DirectX::XMFLOAT3 rotationScale;
		DirectX::XMFLOAT3 posScale;
		float r;

	public:
		Box(Graphics & gfx, const DirectX::XMFLOAT3 & position, const std::string & name, BasicType::ColorFloat material, float rotationR);

		void Update(const DirectX::XMFLOAT3 & delta, const DirectX::XMFLOAT3 & deltaAngle = { 0.0f,0.0f,0.0f }) noexcept override;
		DirectX::XMMATRIX GetTransformMatrix() const noexcept override;
	};
}
